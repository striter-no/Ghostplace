from sys import argv
import requests
import tarfile
import os

if __name__ == "__main__":
    if len(argv) < 2:
        print(f"usage: {argv[0]} DOMAIN http://ROUTER_IP:PORT [optional]API_KEY")
        exit()

    DOMAIN = argv[1]
    ROUTER = argv[2]
    API_KEY = argv[3] if len(argv) > 3 else None

    if not os.path.isdir(DOMAIN):
        print("[!] domain must be directory with site files")
        exit(0)

    with tarfile.open(f"{DOMAIN}.tar", 'w') as arch:
        arch.add(DOMAIN)

    if API_KEY == None:
        api_ans = requests.post(f"{ROUTER}/api")
        if api_ans.text.startswith("denied"):
            print("[!] failed to register API key")
            print(api_ans.text)
            exit(-1)
        print(f"[!] new API key: {api_ans.text}, use it next time in argv")
        API_KEY = api_ans.text

    ans = requests.post(
        f"{ROUTER}/upload",
        params = {"api": API_KEY},
        files = {"file": open(f"{DOMAIN}.tar", "rb")}
    )

    ans_t = ans.text
    if ans.status_code != 200:
        print(f"[!] failed to upload: {ans_t}")
    else:
        print("[*] uploaded")