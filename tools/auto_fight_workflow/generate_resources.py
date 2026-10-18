#!/usr/bin/env python3
"""Generate portable C++ resource metadata from the exact deployment inputs."""

import argparse
import hashlib
import json
from pathlib import Path


def quoted(value):
    # C++ UTF-8 bytes, including names with non-ASCII characters.
    return '"' + ''.join(
        chr(byte) if 32 <= byte < 127 and byte not in (34, 92)
        else '\\\\' if byte == 92 else '\\"' if byte == 34
        else f'\\{byte:03o}'
        for byte in value.encode('utf-8')
    ) + '"'


def sequence(values):
    return '{' + ', '.join(quoted(value) for value in values) + '}'


def classes_from_yaml(path):
    # Preserve BAAS_Yolo_v8::_init_yaml line order, rather than sorting IDs.
    lines = path.read_text(encoding='utf-8').splitlines()
    start = end = 0
    for i, line in enumerate(lines):
        if 'names:' in line:
            start = i + 1
        elif start > 0 and ':' not in line:
            end = i
            break
        if i == len(lines) - 1:
            end = len(lines)
    if not start:
        raise ValueError(f'No names section in {path}')
    return [line.partition(':')[2][1:] for line in lines[start:end]]


def generate(static_path, image_root, model_root, output):
    static = json.loads(static_path.read_text(encoding='utf-8'))
    fight = static['BAAS']['auto_fight']
    for name in (fight['BossHealth']['ocr_model_name'], fight['BattleTime']['ocr_model_name'],
                 fight['Skill']['slot']['cost_ocr']['model_name']):
        if name not in static['ocr_model_name']:
            raise ValueError(f'Unregistered default OCR model: {name}')
    defaults = fight['Skill']['default_templates']
    overrides = fight['Skill']['templates']
    inputs = {"static.json": static_path}
    profiles = {}
    for server, languages in static['server_language_choice'].items():
        for language in languages:
            directory = image_root / server / language
            groups = {}
            for index in sorted((directory / 'image_info').glob('*.json')):
                inputs["images/" + index.relative_to(image_root).as_posix()] = index
                info = json.loads(index.read_text(encoding='utf-8'))
                names = set()
                for name in info['image']:
                    image = directory / info['path'] / (name + '.png')
                    if image.is_file():
                        names.add(name)
                        inputs["images/" + image.relative_to(image_root).as_posix()] = image
                groups.setdefault(info['group'], set()).update(names)
            candidates = set(overrides)
            for names in groups.values():
                candidates.update(names)
            skills = {}
            for name in sorted(candidates):
                mapping = overrides.get(name, {})
                active = [group for group in mapping.get('active', defaults['active'])
                          if name in groups.get(group, set())]
                inactive = [group for group in mapping.get('inactive', defaults['inactive'])
                            if name in groups.get(group, set())]
                if active or inactive:
                    skills[name] = (active, inactive)
            profiles[server + '.' + language] = skills
    models = {}
    for name, config in sorted(fight['yolo']['models'].items()):
        if config['type'] not in ('fp32', 'fp16'):
            raise ValueError(f'Unsupported model precision: {name}')
        yaml_path = model_root / config['yaml_path']
        model_path = model_root / name
        if not model_path.is_file():
            raise FileNotFoundError(model_path)
        inputs["models/" + config["yaml_path"]] = yaml_path
        inputs["models/" + name] = model_path
        models[name] = (config, classes_from_yaml(yaml_path))
    digest = hashlib.sha256()
    # Content-derived identity includes deployed model bytes without embedding them.
    for logical_path, path in sorted(inputs.items()):
        digest.update(logical_path.encode('utf-8') + b'\0')
        content = hashlib.sha256()
        with path.open('rb') as stream:
            while chunk := stream.read(1024 * 1024):
                content.update(chunk)
        digest.update(content.digest())
    health = fight['BossHealth']
    rectangle = lambda values: '{' + ', '.join(str(int(v)) for v in values) + '}'
    rows = [
        '#include "module/auto_fight/workflow/WorkflowResources.h"',
        '',
        'BAAS_NAMESPACE_BEGIN',
        '',
        'const WorkflowResources& workflow_resources()',
        '{',
        '    static const WorkflowResources resources{',
        f'        .identity = "{digest.hexdigest()}",',
        '        .server_languages = {',
    ]
    for server, languages in static['server_language_choice'].items():
        rows.append(f'            {{{quoted(server)}, {sequence(languages)}}},')
    rows.extend(('        },', '        .skills = {'))
    for profile, skills in profiles.items():
        rows.append(f'            {{{quoted(profile)}, {{')
        for name, (active, inactive) in skills.items():
            rows.append(f'                {{{quoted(name)}, {{{sequence(active)}, {sequence(inactive)}}}}},')
        rows.append('            }},')
    rows.extend(('        },', '        .models = {'))
    for name, (config, names) in models.items():
        rows.append(f'            {{{quoted(name)}, {{{quoted(config["type"])}, '
                    f'{quoted(config["yaml_path"])}, {sequence(names)}}}}},')
    rows.extend((
        '        },',
        f'        .ocr_models = {sequence(sorted(static["ocr_model_name"]))},',
        '        .skill_cost_regions = {' + ', '.join(rectangle(r) for r in fight['Skill']['slot']['cost_ocr']['regions']) + '},',
        f'        .skill_cost_ocr_model = {quoted(fight["Skill"]["slot"]["cost_ocr"]["model_name"])},',
        '        .boss_health = {',
        f'            .current_region = {rectangle(health["current_ocr_region"])},',
        f'            .max_region = {rectangle(health["max_ocr_region"])},',
        f'            .region = {rectangle(health["ocr_region"])},',
        f'            .model = {quoted(health["ocr_model_name"])},',
        '        },',
        '        .time_regions = {' + ', '.join(rectangle(r) for r in fight['BattleTime']['ocr_region']) + '},',
        f'        .time_ocr_model = {quoted(fight["BattleTime"]["ocr_model_name"])},',
        # The historical default reads the BossHealth rectangle as uint8, falling back to zero.
        '        .default_time_format = 0,',
        '    };',
        '    return resources;',
        '}',
        '',
        'BAAS_NAMESPACE_END',
        '',
    ))
    output.parent.mkdir(parents=True, exist_ok=True)
    payload = '\n'.join(rows)
    if not output.exists() or output.read_text(encoding='utf-8') != payload:
        output.write_text(payload, encoding='utf-8', newline='\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--static', required=True, type=Path)
    parser.add_argument('--images', required=True, type=Path)
    parser.add_argument('--models', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    generate(args.static, args.images, args.models, args.output)


if __name__ == '__main__':
    main()
