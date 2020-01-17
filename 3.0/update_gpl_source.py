import io
import os
import argparse
import git  # GitPython module
import hashlib
from shutil import copyfile
import pathlib


class PublicFiles(object):
    def __init__(self, git_path="."):
        files = []

        f = open(git_path + '/filelist.txt', 'r')
        x = f.readlines()

        for l in x:
            files.append(l.strip().lower())

        self.files = set(files)
        self.git_path = git_path

    def is_public(self, file, **kwargs):
        if file in self.files:
            return True

        ff = file.split("/")

        for i in range(2, len(ff) + 1):
            path = "/".join(ff[0:i])
            path = path + "/" if self.is_dir(path, **kwargs) else path
            if path in self.files:
                return True

        return False

    def is_dir(self, p, **kwargs):
        git_path = kwargs.get('git_path', self.git_path)
        return os.path.isdir(git_path + p)


class FileList(object):
    def __init__(self, source_path="."):
        self.git_repo = git.Repo(source_path, search_parent_directories=True)
        self.home_path = source_path.replace(".\\", "/").replace("\\", "/").strip("/")
        self.files = {}
        for (dirpath, dirnames, filenames) in os.walk(source_path):
            path = dirpath.replace(".\\", "/").replace("\\", "/")
            for f in filenames:
                if path == ".":
                    self.append_file("/" + f)
                else:
                    self.append_file(path + "/" + f)

    def git_sha(self):
        return self.git_repo.head.reference.commit.hexsha

    def append_file(self, f):
        wo_home = self.remove_home_part(f)
        self.files[wo_home.lower()] = wo_home

    def remove_home_part(self, f):
        if f[:len(self.home_path)].lower() == self.home_path.lower():
            return f[len(self.home_path):]
        return f

    def get_files(self):
        return set(self.files.keys())

    def original_name(self, f):
        if f in self.files:
            return self.files[f].replace("/", "\\")
        else:
            return f.replace("/", "\\")

    def absolute_path(self, f):
        return os.path.abspath(self.home_path + self.original_name(f))

    def is_dir(self, p):
        return os.path.isdir(self.absolute_path(p))

    def remove_file(self, f):
        f = self.original_name(f)
        print("Remove: {}".format(f))
        try:
            self.git_repo.index.remove([self.absolute_path(f)])
            os.remove(self.absolute_path(f))
        except git.exc.GitCommandError:
            os.remove(self.absolute_path(f))

    def file_content_hash(self, filename):
        try:
            with open(self.absolute_path(filename), 'rb') as f:
                h = hashlib.sha1()
                for chunk in iter(lambda: f.read(io.DEFAULT_BUFFER_SIZE), b""):
                    h.update(chunk)
                return h.hexdigest()
        except IOError:
            return "no-file"


def main(gpl_path, internal_path):

    public_files = PublicFiles(gpl_path)
    gpl_files = FileList(gpl_path)
    internal_files = FileList(internal_path)

    removed_counter = 0
    updated_counter = 0
    dont_changed_counter = 0

    print("GPL head: {}".format(gpl_files.git_sha()))
    print("Internal head: {}".format(internal_files.git_sha()))
    print()

    for i in internal_files.get_files():
        if public_files.is_public(i, git_path=internal_files.home_path):
            if gpl_files.file_content_hash(i) != internal_files.file_content_hash(i):
                print("Updating {}".format(internal_files.original_name(i)))
                dst = gpl_files.absolute_path(internal_files.original_name(i))
                pathlib.Path(os.path.dirname(dst)).mkdir(parents=True, exist_ok=True)
                copyfile(internal_files.absolute_path(i), dst)
                # gpl_files.git_repo.index.add([dst])
                updated_counter += 1
            else:
                dont_changed_counter += 1

    for f in gpl_files.get_files():
        # Удаляем файлы GPL отсутствующие в Internal
        if f not in internal_files.files and f not in public_files.files:
            gpl_files.remove_file(f)
            removed_counter += 1
        # Удаляем файлы не подходящие по списку
        elif not public_files.is_public(f):
            gpl_files.remove_file(f)
            removed_counter += 1

    print()
    print("Files not changed: {}".format(dont_changed_counter))
    print("Files removed: {}".format(removed_counter))
    print("Files updated: {}".format(updated_counter))

    if removed_counter or updated_counter:
        message = "refs #1 Synchronize with {branch} commit: {c}".format(branch="release", c=internal_files.git_sha())
        print("Sources changed. Please review changes, commit and push.")
        print("Cooment: {}".format(message))
        # commit = gpl_files.git_repo.index.commit(message)
        #print("Commit GPL repo: {}".format(commit.hexsha))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Terminal Client GLP source updater')
    parser.add_argument('--gpl', type=str, default= '.', help='GPL source git repo path')
    parser.add_argument('--internal', type=str, required=True, help='Internal source git repo path')
    args = vars(parser.parse_args())
    main(args['gpl'], args['internal'])


