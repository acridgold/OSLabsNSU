# Как запускать и чего ждать

```bash
cd ~/lab6
echo "Это секретные данные!" > secret.txt
chmod 400 secret.txt
````
---

~/lab6
❯ `./prog`
Real UID: 1000
Effective UID: 1000
Содержимое файла: Это секретные данные!

~/lab6
❯ `sudo -u delete_me ./prog`
Real UID: 1001
Effective UID: 1001
Ошибка открытия файла: Permission denied

~/lab6
❯
``` bash
# Устанавливаем владельца
sudo chown root prog
# Устанавливаем SUID-бит
sudo chmod 4755 prog
```


~/lab6
❯ `sudo -u delete_me ./prog`
Real UID: 1001
Effective UID: 0
Содержимое файла: Это секретные данные!

~/lab6
❯ `./prog`
Real UID: 1000
Effective UID: 0
Содержимое файла: Это секретные данные!

---