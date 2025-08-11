from sys import argv
from pathlib import Path
from flask import Flask, request, jsonify
from werkzeug.utils import secure_filename
import tarfile
import pysrc.database as db
import uuid, hashlib

# Настройки
UPLOAD_DATABASE = db.DataBase("./assets/databases/uploads.sqlite3")
USERS_DATABASE = db.DataBase("./assets/databases/users.sqlite3")

UPLOAD_FOLDER = Path("./assets/sites")
UPLOAD_FOLDER.mkdir(exist_ok=True)

app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 100 * 1024 * 1024  # 100 MB limit

# Допустимые расширения
ALLOWED_EXTENSIONS = {'tar'}


def is_domain_taken(domain: str) -> bool:
    for k in UPLOAD_DATABASE.all():
        if domain in UPLOAD_DATABASE.get(k)["domains"]:
            return True
    return False

def gen_api_key():
    return str(uuid.uuid4())

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def unpack_tar(archive_path, extract_to) -> bool:
    with tarfile.open(archive_path, 'r:*') as tar:
        if not tar.getmembers()[0].isdir():
            return False

        # Безопасная распаковка
        for member in tar.getmembers():
            print(f"member: {member.path}")
            member_path = Path(extract_to) / member.name
            if not member_path.resolve().is_relative_to(extract_to.resolve()):
                raise ValueError(f"Attempt to escape extraction directory: {member.name}")
        tar.extractall(path=extract_to)
    return True

@app.route('/api', methods=['POST'])
def api_router():
    if USERS_DATABASE.get(request.remote_addr) is None:
        api = gen_api_key()
        print(f"[WARNING] New API will be registered from {request.remote_addr} ({api})")
        
        USERS_DATABASE.set(request.remote_addr, {
            "api": hashlib.sha256(api.encode()).hexdigest()
        })

        UPLOAD_DATABASE.set(hashlib.sha256(api.encode()).hexdigest(), {
            "domains": []
        })

        return api, 200
    else:
        return "denied: cannot register more than one API key from one IP", 422

@app.route('/upload', methods=['POST'])
def upload_archive():
    row_api = request.args.get("api")
    if row_api is None:
        return "no api provided in params", 400
    
    api = hashlib.sha256(row_api.encode()).hexdigest()
    ldata = UPLOAD_DATABASE.get(api)
    if ldata is None:
        return "api key is not registered", 403

    local_domains = ldata["domains"]

    if 'file' not in request.files:
        return "no file part", 400

    file = request.files['file']

    if file.filename == '':
        return "no selected file", 400

    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        archive_name = Path(filename).stem  # имя без расширения
        
        if is_domain_taken(archive_name):
            return "domain taken", 403
        
        local_domains.append(archive_name)

        target_dir = UPLOAD_FOLDER
        target_dir.mkdir(exist_ok=True)

        print(f"[WARNING] Incoming site from {request.remote_addr} (named: {archive_name[:100]})")
        # allow = input("allow to upload? [Y/n]") or "Y"
        # if (allow != "Y"):
        #     return "denied", 403

        temp_path = target_dir / filename
        file.save(temp_path) 

        try:
            print(filename)
            
            status = unpack_tar(
                temp_path,
                target_dir
            )
            temp_path.unlink()

            if status:
                UPLOAD_DATABASE.set(api, local_domains)
                return "ok", 200
            else:
                return "archive content doesn't match with expected", 400
        except Exception as e:
            return "internal error", 500
    else:
        return "invalid file type - only *.tar allowed", 400

if __name__ == '__main__':
    app.run(host=argv[1], port=int(argv[2]), debug=False)