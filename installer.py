import logging
import os.path
import shutil
import subprocess
import traceback
import zipfile


TMP_PATH = './tmp'                                                              # gitee的下载地址需要把blob改成raw
REPO_URL_HTTP = 'https://gitee.com/pur1fy/i-saki-assistant-compiled-files.git'  # 此处填写要克隆仓库的地址
GIT_HOME = './toolkit/Git/bin/git.exe'                                          # 需要修改为git.exe的路径
LOCAL_PATH = './i-saki-assistant-compiled-files'                                # 需要修改为仓库名

logger = logging.getLogger()
logger.setLevel(logging.INFO)
console = logging.StreamHandler()
console.setLevel(logging.INFO)

formatter = logging.Formatter("%(asctime)s - %(levelname)s: %(message)s")
console.setFormatter(formatter)
logger.addHandler(console)


def mv_repo(folder_path: str):
    for item in os.listdir(folder_path):
        item_path = os.path.join(folder_path, item)
        shutil.move(item_path, './')


def unzip_file(zip_dir, out_dir):
    with zipfile.ZipFile(zip_dir, 'r') as zip_ref:
        # 解压缩所有文件到当前目录
        zip_ref.extractall(path=out_dir)
        logger.info(f"{zip_dir} unzip success, output files in {out_dir}")


def check_git():
    logger.info("Checking git installation...")
    if not os.path.exists('./.git'):
        logger.info("+--------------------------------+")
        logger.info("|         INSTALL ISA            |")
        logger.info("+--------------------------------+")
        subprocess.run([GIT_HOME, 'clone', '--depth', '1', REPO_URL_HTTP])
        mv_repo(LOCAL_PATH)
        logger.info("Install success")
    elif not os.path.exists('./no_update'):
        logger.info("+--------------------------------+")
        logger.info("|          UPDATE ISA            |")
        logger.info("+--------------------------------+")
        remote_sha = (subprocess.check_output([GIT_HOME, 'ls-remote', '--heads', 'origin', 'refs/heads/main'])
                      .decode('utf-8')).split('\t')[0]
        local_sha = (subprocess.check_output([GIT_HOME, 'rev-parse', 'HEAD'])
                     .decode('utf-8')).split('\n')[0]
        logger.info(f"remote_sha:{remote_sha}")
        logger.info(f"local_sha:{local_sha}")
        if local_sha == remote_sha and subprocess.check_output([GIT_HOME, 'diff']) == b'':
            logger.info("No updates available")
        else:
            logger.info("Pulling updates from the remote repository...")
            subprocess.run([GIT_HOME, 'reset', '--hard', 'HEAD'])
            subprocess.run([GIT_HOME, 'pull', REPO_URL_HTTP])

            updated_local_sha = (subprocess.check_output([GIT_HOME, 'rev-parse', 'HEAD'])
                                 .decode('utf-8')).split('\n')[0]
            if updated_local_sha == remote_sha:
                logger.info("Update success")
            else:
                logger.warning("Failed to update the source code, please check your network or for conflicting files")


def create_tmp():
    if not os.path.exists(TMP_PATH):
        os.mkdir(TMP_PATH)
    if not os.path.exists(LOCAL_PATH):
        os.mkdir(LOCAL_PATH)


def clear_tmp():
    if os.path.exists(TMP_PATH):
        shutil.rmtree(TMP_PATH)
    if os.path.exists(LOCAL_PATH):
        shutil.rmtree(LOCAL_PATH)


def check_install():
    try:
        clear_tmp()
        create_tmp()
        check_git()
    except Exception as e:
        traceback.print_exc()
        clear_tmp()
        os.system('pause')


if __name__ == '__main__':
    check_install()
    logger.info("Press any key to exit...")
    os.system('pause')
