# Volumio２のステイタスが変更された時（プレイ・一時停止、など）アルバムアートを取得してリモコン用にサイズ変更するプログラム（Python3）

# 参考
# https://gist.github.com/ivesdebruycker/4b08bdd5415609ce95e597c1d28e9b9e
# https://volumio.org/forum/gpio-pins-control-volume-t2219.html
# https://pypi.python.org/pypi/socketIO-client
# https://volumio.github.io/docs/API/WebSocket_APIs.html

# Web取得関係
# https://note.nkmk.me/python-download-web-images/

# 画像処理（PIL利用リサイズ）
# https://note.nkmk.me/python-pillow-image-resize/

import time
import subprocess
from socketIO_client import SocketIO, LoggingNamespace
import urllib.error
import urllib.request
from PIL import Image, ImageFilter

VOLUMIO_IP = '192.168.1.86'

TARGET_SIZE = (350,350)

socketIO = SocketIO(VOLUMIO_IP, 3000)
status = 'pause'

def download_file(url, dst_path):
    try:
        with urllib.request.urlopen(url) as web_file, open(dst_path, 'wb') as local_file:
            local_file.write(web_file.read())
    except urllib.error.URLError as e:
        print(e)


def on_push_state(*args):
		print('state', args)
		global status
		status = args[0]['status'].encode('ascii', 'ignore')
		print (status)
		albm = "http://"+VOLUMIO_IP+args[0]['albumart']
		print (albm)
		url = (albm)
		#dst_path = '/home/pi/v_temp/image.png'
		dst_path = 'in_image.png'
		download_file(url, dst_path)

		im = Image.open('in_image.png')
		# print(im.format, im.size, im.mode)
		img_resize = im.resize(TARGET_SIZE)
		img_resize.save('resize.png')


socketIO.on('pushState', on_push_state)

# get initial state
socketIO.emit('getState', '', on_push_state)


try:
	socketIO.wait()
except KeyboardInterrupt:
	pass
