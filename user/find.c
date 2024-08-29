#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

#define BUF_SIZE 512

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void find(char *path, char *filename) {
    char buf[512]; // buffer to store the path
    char *p; // pointer to manipulate string
    int fd; // file directory
    struct dirent de; // directory entry
    struct stat st;

    // open the file/dir
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // get the status of the file/dir
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // with file opened and file status prepared
    switch ((st.type))
    {
        case T_FILE:
            if (strcmp(fmtname(path), filename) == 0) {
                printf("%s\n", path);  
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("ls: path is too long.\n");
                break;
            }
            // prepare to read dir entries
            strcpy(buf, path); // copy path to buffer
            p = buf + strlen(buf); // pointer move the end of the path
            *p++ = '/'; // append slash

            // read directory entries
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                // skip the empty dir
                if (de.inum == 0) {
                    continue;
                }

                // skip parent and current dir
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                {
                    continue;
                }

                // copy directory name to where pointer p points to
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;

                if (stat(buf, &st) < 0) {
                    fprintf(2, "find: cannot stat %s\n", buf);
                    continue;
                }

                // for each directory entry, continue to check
                if (st.type == T_FILE) {
                    // if this entry is file, print it
                    if (strcmp(de.name, filename) == 0) {
                        printf("%s\n", buf);
                    } 
                } else if (st.type == T_DIR) {
                    // if this entry is directory, recurse into next level
                    find(buf, filename);
                }
            }
            break;
    }
    close(fd);
        
}

int main(int argc, char *argv[]) {
    // check number of arguments
    if (argc != 3) {
        fprintf(2, "Usage: find <filepath> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);

}