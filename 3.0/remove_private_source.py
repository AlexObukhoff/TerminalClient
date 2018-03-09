import io
import os

# Windows filesystem ONLY 
# for linux remove .lower() calls


def get_public_filenames():
	f = open('filelist.txt', 'r')
	x = f.readlines()

	result = []
	
	for l in x:
		result.append(l.strip().lower())

	return result

	
def get_all_files():
	files = []
	for (dirpath, dirnames, filenames) in os.walk("."):
		path = dirpath.replace(".\\", "/").replace("\\", "/")
		for f in filenames:
			if path == ".":
				files.append("/" + f.lower())
			else:
				files.append(path.lower() + "/" + f.lower())
	return files

	
def is_dir(p):
	return os.path.isdir(os.getcwd() + p.replace("/", "\\"))



	
public_files = set(get_public_filenames())
removed_counter = 0

for f in get_all_files():

	is_public = f in public_files
	
	if is_public:
		continue
	
	ff = f.split("/")
	
	for i in range(2, len(ff) + 1):
		path = "/".join(ff[0:i])
		path = path + "/" if is_dir(path) else path
		is_public = is_public or (path in public_files)
		
		if is_public:
			break
	
	if is_public:
		continue
		
	os.remove(os.getcwd() + f.replace("/", "\\"))
	removed_counter += 1

print("Files removed: {}".format(removed_counter))

