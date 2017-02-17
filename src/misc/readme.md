Install the script in crontab with @reboot parameter

for rpi-slave crontab in user root:
@reboot /opt/skripsi-script/lowpan_if.sh 0x002 fe80::1:2/64


for rpi-master crontab in user pi:
@reboot nohup /opt/ngrok/start-telegram.sh > /opt/ngrok/startup_log &
