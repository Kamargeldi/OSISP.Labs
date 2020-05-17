#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

static PPMImage *readPPM(char* bytes, char* color, int len, int picnum)
{
    PPMImage *img;
    int rgb_comp_color, pos = 3;

    //check the image format
    if (bytes[0] != 'P' || bytes[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    while (bytes[pos] == '#') 
    {
        while (bytes[pos] != '\n')
        {
            pos++;
        }
        pos++;
    }

    //read image size information
    char size[10]; size[0] = '\0';
    int counter = 0;
    while (bytes[pos] != ' ')
    {
        size[counter] = bytes[pos];
        counter++;
        size[counter] = '\0';
        pos++;
    }
    
    img->x = atoi(size);
    pos++;
    counter = 0;
    size[0] = '\0';

    while (bytes[pos] != '\n')
    {
        size[counter] = bytes[pos];
        counter++;
        size[counter] = '\0';
        pos++;
    }

    img->y = atoi(size);
    pos++;
    counter = 0;
    size[0] = '\0';

    //read rgb_component_color    

    while (bytes[pos] != '\n')
    {
        size[counter] = bytes[pos];
        counter++;
        size[counter] = '\0';
        pos++;
    }

    rgb_comp_color = atoi(size);

    //check rgb component depth
    if (rgb_comp_color != RGB_COMPONENT_COLOR) 
    {
         fprintf(stderr, "File does not have 8-bits components\n");
         exit(1);
    }

    pos++;
    
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    while (pos < img->x * img->y * 3)
    {
        if (!strcmp(color, "red"))
        {
            bytes[pos + 1] = 0;
            bytes[pos + 2] = 0;
        }
        else if (!strcmp(color, "green"))
        {
            bytes[pos] = 0;
            bytes[pos + 2] = 0;
        }
        else
        {
            bytes[pos + 1] = 0;
            bytes[pos] = 0;
        }
        
        pos += 3;
    }
    
    char filename[256] = "/home/kamar/Desktop/ppmResult/picture";
    char picn[10];
    sprintf(picn, "%d", picnum);
    strcat(filename, picn);
    strcat(filename, "_");
    strcat(filename, color);
    strcat(filename, "_channel.ppm");
    FILE* fp = fopen(filename, "wb");
    fwrite(bytes, 1, len, fp);
    return img;
}





int main(int argc, char* argv[])
{
    int fdR[2];
    int fdG[2];
    int fdB[2];
    size_t size;
    int waitStat;
    int pidParent = getpid();
    int pidR = 0;
    int pidG = 0;
    int pidB = 0;

    if (pipe(fdR) < 0)
    {
        printf("Error on creating pipe\n");
    } 
    if (pipe(fdG) < 0)
    {
        printf("Error on creating pipe\n");
    } 
    if (pipe(fdB) < 0)
    {
        printf("Error on creating pipe\n");
    }
        
    pidR = fork();
    if (pidR > 0)
    {
        pidG = fork();
        if (pidG > 0)
        {
            pidB = fork();
            if (pidB > 0)
            {

            }
            else if (pidB == 0)
            {
                // BLUE COLOR PROCESS
                int picnum = 0;
                while(1)
                {
                    int len;
                    read(fdB[0], &len, sizeof(int));
                    char* getstring = (char*)malloc(len * sizeof(char));
                    size_t get_size = 0;
                    char c;
                    int cur = 0;
                    while (get_size != len)
                    {
                        get_size += read(fdB[0], &getstring[cur], sizeof(char));
                        cur++;
                    }

                    PPMImage* image = readPPM(getstring, "blue", len, picnum);
                    //printf("PPMImage height: %d   width:%d \n", image->x, image->y);
                    free(getstring);
                    picnum++;
                }
            }
            else
            {
                printf("Error occured pidB\n");
            }
        }
        else if (pidG == 0)
        {
            // GREEN COLOR PROCESS
            int picnum = 0;
            while(1)
            {
                int len;
                read(fdG[0], &len, sizeof(int));
                char* getstring = (char*)malloc(len * sizeof(char));
                size_t get_size = 0;
                char c;
                int cur = 0;
                while (get_size != len)
                {
                    get_size += read(fdG[0], &getstring[cur], sizeof(char));
                    cur++;
                }

                PPMImage* image = readPPM(getstring, "green", len, picnum);
                //printf("PPMImage height: %d   width:%d \n", image->x, image->y);
                free(getstring);
                picnum++;
            }
        }
        else
        {
            printf("Error occured pidG\n");
        }
    }
    else if (pidR == 0)
    {
        // RED COLOR PROCESS
        int picnum = 0;
        while(1)
        {
            int len;
            read(fdR[0], &len, sizeof(int));
            char* getstring = (char*)malloc(len * sizeof(char));
            size_t get_size = 0;
            char c;
            int cur = 0;
            while (get_size != len)
            {
                get_size += read(fdR[0], &getstring[cur], sizeof(char));
                cur++;
            }

            PPMImage* image = readPPM(getstring, "red", len, picnum);
            //printf("PPMImage height: %d   width:%d \n", image->x, image->y);
            free(getstring);
            picnum++;
        }
    }
    else
    {
        printf("Error occured pidR\n");
    }


    if (getpid() == pidParent)
    {
        DIR* dir = opendir(argv[1]);
        struct dirent* directory;
        directory = readdir(dir);
        
         
        while (directory != NULL)
        {
            if (directory->d_type == DT_REG)
            {
                char fname[256] = "\0";
                strcat(fname, argv[1]);
                strcat(fname, directory->d_name);
                printf("%s\n", fname);
                FILE* f = fopen(fname, "rb");
                fseek(f, 0, SEEK_END);
                int filelen = ftell(f);
                rewind(f);
                char* buffer = (char*)malloc(filelen * sizeof(char));
                fread(buffer, filelen, 1, f);
                fclose(f);

                size = write(fdR[1], &filelen, sizeof(int));
                size = write(fdR[1], (char*)buffer, filelen);
                size = write(fdG[1], &filelen, sizeof(int));
                size = write(fdG[1], (char*)buffer, filelen);
                size = write(fdB[1], &filelen, sizeof(int));
                size = write(fdB[1], (char*)buffer, filelen);

                free(buffer);
            }

            directory = readdir(dir);
        }

        closedir(dir);
        sleep(3);
        kill(pidR, SIGKILL);
        kill(pidG, SIGKILL);
        kill(pidB, SIGKILL);
        for (int i = 0; i < 3; i++)
        {
            wait(&waitStat);
        }

        close(fdR[0]);
        close(fdR[1]);
        close(fdG[0]);
        close(fdG[1]);
        close(fdB[0]);
        close(fdB[1]);
    }
    
    return 0;
}