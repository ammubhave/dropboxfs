import os
import sys
import fcntl
from struct import *
import ctypes
import dropbox
import time

app_key = '<APP_KEY>'
app_secret = '<APP_SECRET>'
access_token = '<ACCESS_TOKEN>'
client = dropbox.client.DropboxClient(access_token)

logical_block_size = 4096; # 4MB

req_type = sys.argv[1]
sector = long(sys.argv[2])
nsect = long(sys.argv[3])

#os.system("echo " + req_type + " >> /home/ambhave/dropboxfs/test")

if (req_type == 'r'):
	key = long(sys.argv[4], 16)
	buf = None#ctypes.create_string_buffer(logical_block_size * nsect)# ctypes.create_string_buffer("Hello my friend", logical_block_size * nsect)
	s = pack('P', key)#, ctypes.addressof(buf))


	os.system("echo Want to read " + str(sector) + " nsect  " + str(nsect) + " >> /home/ambhave/dropboxfs/test")
	try:
		#for i in range(sector, sector + nsect):
			#os.system("echo Reading " + str(i) + " >> /home/ambhave/dropboxfs/test")

		flock = open('/home/ambhave/dropboxfs/blocks/' + str(sector), 'a+')
		while True:
			try:
				fcntl.lockf(flock, fcntl.LOCK_EX | fcntl.LOCK_NB)
			except IOError:
				time.sleep(1)
				continue
			break

		while True:
			try:
				with client.get_file('/dbxfs/blocks/' + str(sector) + ".txt") as f:
					buf = ctypes.create_string_buffer(f.read(), logical_block_size)
			except dropbox.rest.ErrorResponse:
				buf = ctypes.create_string_buffer(logical_block_size)
			except Exception, ex:
				continue
			break

		flock.close()
		print repr(buf.raw)

		filename = "/proc/dropbox0"
		fd = os.open(filename, os.O_SYNC | os.O_WRONLY)
		os.write(fd, bytearray(s) + bytearray(buf))
			#IOCTL_SEND_DATA_DBX = ((2 << (30)) | (251 << 8) | (8 << (16)))
		#os.system("echo opening... >> /home/ambhave/dropboxfs/test")
	except Exception, ex:
		f = open('/home/ambhave/dropboxfs/test', 'a')
		f.write('ERROR 1 : ')
		f.write(str(ex))
		f.write('\n')
		f.close()
		#f.write((bytearray(data)));
		#f.write(" ");
		#os.system("echo opened >> /home/ambhave/dropboxfs/test")

		#a = (hex(key)[2:-1]).zfill(16)
		#b = (hex(ctypes.addressof(buf))[2:]).zfill(16)

		#f = open('/home/ambhave/dropboxfs/test', 'a')
		#f.write(bytearray(s) + bytearray(buf));
		#f.close()

		#os.system("echo -n -e \\\\x" + a[14:15] + "\\\\x" + a[12:13] + "\\\\x" + a[10:11] + "\\\\x" + a[8:9]
		#                   + "\\\\x" + a[6:7] + "\\\\x" + a[4:5] + "\\\\x" + a[2:3] + "\\\\x" + a[0:1]
		#		           + "\\\\x" + b[14:15] + "\\\\x" + b[12:13] + "\\\\x" + b[10:11] + "\\\\x" + b[8:9]
		#                   + "\\\\x" + b[6:7] + "\\\\x" + b[4:5] + "\\\\x" + b[2:3] + "\\\\x" + b[0:1] + " > /proc/dropbox0")
		#os.system("echo -n -e \\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff\\\\xff > /proc/dropbox0")
		#ioctl(fd, dmesg, s)
		#os.system("echo closing... >> /home/ambhave/dropboxfs/test")
		#fd.close()
		#os.system("echo closed >> /home/ambhave/dropboxfs/test")
else:
	try:
		data = sys.argv[4]
		#os.system("echo data len is " + str(len(data)) + " >> /home/ambhave/dropboxfs/test")
		#f = open('/home/ambhave/dropboxfs/test', 'a')
		#f.write((bytearray(data)));
		#f.write(" ");
		data = data.replace('\\0', chr(0)).replace('\\\\', '\\')

		os.system("echo nsect is " + str(nsect) + " >> /home/ambhave/dropboxfs/test")

		#f.write(repr(bytearray(data)));
		#f.write("\n");
		#f.close()

		if not os.path.exists('/home/ambhave/dropboxfs/blocks/' + str(sector)):
			open('/home/ambhave/dropboxfs/blocks/' + str(sector), 'w').close()

		f = open('/home/ambhave/dropboxfs/blocks/' + str(sector), 'r+')
		while True:
			try:
				fcntl.lockf(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
			except IOError:
				time.sleep(1)
				continue
			break
		f.write(bytearray(data)[:logical_block_size])
		while True:
			try:
				f.seek(0)
				response = client.put_file('/dbxfs/blocks/' + str(sector) + ".txt", f, overwrite=True)
			except dropbox.rest.ErrorResponse, ex:
				if ex.status == 503:
					f3 = open('/home/ambhave/dropboxfs/test', 'a')
					f3.write('ERROR 503 : ')
					f3.write(str(ex) + " " + str(sector) + " " + str(nsect))
					f3.write('\n')
					f3.close()
					time.sleep(1)
					continue
			break
		f2 = open('/home/ambhave/dropboxfs/test', 'a')
		f2.write(str(response))
		f2.write("\n");
		f2.close()

		f.close()
	except Exception, ex:
		f = open('/home/ambhave/dropboxfs/test', 'a')
		f.write('ERROR 1 : ')
		f.write(str(ex) + " " + str(sector) + " " + str(nsect))
		f.write('\n')
		f.close()



# f = open("/home/ambhave/dropboxfs/test", "a")
# f.write(repr(sys.argv))
# f.write(s)
# f.write('\n')
# f.close()

#os.system("echo \"" + sys.argv[0] + "\" > /home/ambhave/dropboxfs/test")ffff88007bd72d00
