#!/usr/bin/env python3
"""Exercise metadata extraction with independent resource fixtures."""

import hashlib
import importlib.util
import json
import shutil
import sys
import tempfile
from pathlib import Path


def load_module(path):
    spec = importlib.util.spec_from_file_location("baas_resource_generator", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    root = Path(sys.argv[1]).resolve()
    module = load_module(root / "tools/auto_fight_workflow/generate_resources.py")
    with tempfile.TemporaryDirectory(prefix="baas-resource-fixture-") as directory:
        fixture = Path(directory).resolve()
        models = fixture / "models"
        models.mkdir()
        (models / "m.onnx").write_bytes(b"fixture model bytes")
        (models / "m.yaml").write_text("names:\n  9: second\n  1: first\n", encoding="utf-8")
        static = {
            "server_language_choice": {"CN": ["zh-cn"], "JP": ["ja-jp"]},
            "ocr_model_name": {"en-us": {}},
            "BAAS": {"auto_fight": {
                "Skill": {"default_templates": {"active": ["active"], "inactive": ["inactive"]},
                          "templates": {"Mika": {"active": ["special"]}},
                          "slot": {"cost_ocr": {"regions": [[0, 1, 2, 3]], "model_name": "en-us"}}},
                "yolo": {"models": {"m.onnx": {"type": "fp32", "yaml_path": "m.yaml"}}},
                "BossHealth": {"current_ocr_region": [0, 1, 2, 3], "max_ocr_region": [4, 5, 6, 7],
                               "ocr_region": [0, 1, 6, 7], "ocr_model_name": "en-us"},
                "BattleTime": {"ocr_region": [[0, 1, 2, 3], [4, 5, 6, 7]], "ocr_model_name": "en-us"},
            }},
        }
        static_path = fixture / "static.json"
        static_path.write_text(json.dumps(static), encoding="utf-8")
        images = fixture / "images"
        profile = images / "CN" / "zh-cn"
        (profile / "image_info").mkdir(parents=True)
        for group, names in (("active", ["Mika"]), ("special", ["Mika", "Missing"]), ("inactive", ["OnlyInactive"])):
            (profile / group).mkdir()
            (profile / "image_info" / (group + ".json")).write_text(
                json.dumps({"group": group, "path": group, "image": {name: {} for name in names}}),
                encoding="utf-8")
            for name in names:
                if name != "Missing":
                    (profile / group / (name + ".png")).write_bytes(b"fixture template")
        output = fixture / "WorkflowResources.cpp"
        module.generate(static_path, images, models, output)
        before = output.read_text(encoding="utf-8")
        assert '{"Mika", {{"special"}, {}}}' in before
        assert '{"OnlyInactive", {{}, {"inactive"}}}' in before
        assert '"Missing"' not in before
        assert '"second", "first"' in before, "Class line order must be retained"
        assert '{"JP.ja-jp", {' in before
        module.generate(static_path, images, models, output)
        assert output.read_text(encoding="utf-8") == before, "Generation must be deterministic"
        relocated = fixture / "relocated-models"
        shutil.copytree(models, relocated)
        module.generate(static_path, images, relocated, output)
        assert output.read_text(encoding="utf-8") == before, "Identity must not depend on installation path"
        (models / "m.onnx").write_bytes(b"new model version")
        module.generate(static_path, images, models, output)
        assert output.read_text(encoding="utf-8") != before, "Model update must change identity"
        assert module.quoted('a"b\\c') == '"a\\"b\\\\c"'
    # Frozen behavioral evidence stays untouched, including its recorded digest.
    for folder in ("base_oracle", "condition_base_oracle", "workflow_base_oracle"):
        for provenance in (root / "tests/auto_fight_workflow" / folder).glob("*.provenance.json"):
            info = json.loads(provenance.read_text(encoding="utf-8"))
            payload = provenance.with_name(provenance.name.replace(".provenance", "")).read_bytes()
            candidates = {hashlib.sha256(payload).hexdigest(),
                          hashlib.sha256(payload.replace(b"\r\n", b"\n")).hexdigest()}
            expected = info.get("output_sha256")
            if expected:
                assert expected in candidates, provenance
    print("Resource profiles, template overrides, ordered classes and frozen oracle hashes passed.")


if __name__ == "__main__":
    main()
