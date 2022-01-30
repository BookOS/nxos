import os
import time
import sys

def hex2str16(filepath):
    fileinfo = os.stat(filepath)
    line = fileinfo.st_size / 16
    target = ""
    f = open(filepath, "rb")
    for l in range(int(line)):
        data = f.read(16)
        he = "    "
        for da in data:
            he = he + hex(da) + ","
        target = target + he + "\n"
        print("\rfile2c: Rate of progress: %f%% " % (l *100 / line), end="")
    f.close()
    return target

def save2content(filename, data):
    path = filename + ".c"
    file = open(path, "w")
    file.write(data)
    file.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("file2c: argv must >= 2")
        exit()

    if os.path.exists(sys.argv[1]) is False:
        print("file2c: file not exit")
        exit()

    try:
        wr_path = sys.argv[2]
    except:
        wr_path = "."

    if os.path.exists(wr_path) is False:
        print(f"file2c: path:{wr_path} not exit")
        exit()

    start_time = time.time()
    filepath = sys.argv[1]
    print("file2c: The file is:" + sys.argv[1], " write to path:" + wr_path)

    if "\\" in filepath:
        filename = filepath.split("\\")[-1].split(".")[0] 
    elif "/" in filepath:
        filename = filepath.split("/")[-1].split(".")[0] 
    else:
        filename = filepath.split(".")[0]

    content = "unsigned char __hex_file_" + filename + "[] = {\n" 
    content += hex2str16(filepath) + "};\n"
    save2content(wr_path + "/" + filename , content)
    end_time = time.time( )
    print("\nfile2c: Last time:", end="")
    print(end_time - start_time)
