#!/usr/bin/env python3
"""
Создаёт структуру для проверочной:
папки task1..task5 и пустые скрипты.
Входные данные НЕ генерирует — этим занимается selfcheck.py при запуске.

Запуск (из чистой папки):
    python3 generate.py
"""
import sys
import os
from pathlib import Path


SOLUTION_TEMPLATE = "#!/bin/sh\n# Твоё решение здесь\n"

SCRIPT_NAMES = {
    "task1": "welcome.sh",
    "task2": "check_environs.sh",
    "task3": "find_fraud.sh",
    "task4": "ban_manager.sh",
    "task5": "report.sh",
}


def write(path, content, executable=False):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    if executable:
        os.chmod(path, 0o755)


def build():
    root = Path(__file__).parent.resolve()

    existing = [d for d in SCRIPT_NAMES if (root / d).exists()]
    if existing:
        print(f"В текущей директории уже есть: {', '.join(existing)}")
        print("Удали их (cleanup.py) или запусти генератор в чистой папке.")
        sys.exit(1)

    for task, script in SCRIPT_NAMES.items():
        write(root / task / script, SOLUTION_TEMPLATE, executable=True)

    print(f"Структура создана в: {root}")
    print()
    print("Не забудь положить task.md в каждую папку.")
    print("Тестовые данные генерируются автоматически при запуске selfcheck.py.")


if __name__ == "__main__":
    build()
