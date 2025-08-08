import os
import asyncio
from pathlib import Path
from flask import Flask, request, jsonify
from werkzeug.utils import secure_filename
import tarfile

# Настройки
UPLOAD_FOLDER = Path("sites")
UPLOAD_FOLDER.mkdir(exist_ok=True)

app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 100 * 1024 * 1024  # 100 MB limit

# Допустимые расширения
ALLOWED_EXTENSIONS = {'tar'}

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def unpack_tar(archive_path, extract_to):
    with tarfile.open(archive_path, 'r:*') as tar:
        # Безопасная распаковка
        for member in tar.getmembers():
            member_path = Path(extract_to) / member.name
            if not member_path.resolve().is_relative_to(extract_to.resolve()):
                raise ValueError(f"Attempt to escape extraction directory: {member.name}")
        tar.extractall(path=extract_to)

@app.route('/upload', methods=['POST'])
async def upload_archive():
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400

    file = request.files['file']

    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400

    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        archive_name = Path(filename).stem  # имя без расширения
        target_dir = UPLOAD_FOLDER
        target_dir.mkdir(exist_ok=True)

        # Сохраняем архив во временный файл
        temp_path = target_dir / filename
        await asyncio.to_thread(file.save, temp_path)  # асинхронное сохранение

        # Распаковка в отдельном потоке
        try:
            await asyncio.get_event_loop().run_in_executor(
                None,
                unpack_tar,
                temp_path,
                target_dir
            )
            # Опционально: удалить архив после распаковки
            temp_path.unlink()
            return "ok", 200
        except Exception as e:
            return "internal error", 500
    else:
        return "invalid file type - only *.tar allowed", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=9001, debug=False)