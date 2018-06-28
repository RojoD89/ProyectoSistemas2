#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void handle_directories(const char * , const char *, int num, unsigned size_param, int argc, char extensions[][argc] );

const char * add_slash(const char * dir){
	static char path[255];
	strcpy(path, dir);
	int len = strlen(path);
	path[len] = '/';
	path[len+1] = '\0';
	return  path;
}

const char * reformat_path(const char * dir){
	static char path[255];
	if(dir[strlen(dir)-1] != '/'){
		strcpy(path, dir);
		strcpy(path,add_slash(path));
		return path;
	}
	else
		return dir;
}

int copy_file(const char * origin, const char * destiny){
	FILE *ptr_or, *ptr_des;
	int  a;
	ptr_or = fopen(origin, "r");
	ptr_des = fopen(destiny, "w");
	if (ptr_or == NULL)
    {
        printf("Error al abrir archivo %s \n", origin);
        return -1;
    }
    if (ptr_des == NULL)
    {
        printf("Error al abrir archivo %s \n", destiny);
        fclose(ptr_or);
        return -1;
    }

    while( ( a = fgetc(ptr_or) ) != EOF )
      fputc(a, ptr_des);

  	fclose(ptr_or);
    fclose(ptr_des);
 
    return 0;
}

int get_extensions(int argc, char const *argv[], char ext[][argc]){
	int j = 0, num = 0;
	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-x") == 0){
			if(argv[i+1] == NULL){
				printf("El parametro -x debe estar segido por alguna extension.\n");
				exit(1);
			}
			strcpy(ext[j], argv[i+1]);
			j++;
			num++;
		}
	}
	return num;
}

static unsigned long get_size_param(int argc, char const *argv[]){
	unsigned long size = 0;
	char *ptr;

	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-m") == 0){
			if(argv[i+1] == NULL){
				printf("El parametro -x debe estar segido por alguna extension.\n");
				exit(1);
			}
			size = strtol(argv[i+1], &ptr, 10);
			if(size == 0){
				printf("El parametro de tamaño máximo debe ser un número.\n");
				exit(1);
			}
		}
	}
	return size*1000;
}


const char * build_file_name(const char * dir, const char * name){
	static char file_name[255];
	strcpy(file_name, dir);
	strcat(file_name, name);
	return file_name;
}

const char * get_file_extension(const char * name){
	const char * ext = strrchr(name, '.');
	if(!ext || ext == name) return "";
	return ext + 1;
}

static unsigned long get_file_size (const char * file_name)
{
    struct stat sb;
    if (stat (file_name, & sb) != 0) {
        fprintf (stderr, "'stat' failed for '%s': %s.\n",
                 file_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    return sb.st_size;
}

time_t get_last_modified(const char * file_name){
	struct stat sb;
    if (stat (file_name, & sb) != 0) {
        fprintf (stderr, "'stat' failed for '%s': %s.\n",
                 file_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    return sb.st_mtime;
}


int check_extension(const char * file_ext, int num, int argc, char extensions[][argc]){
	int n = 0;
	if(strcmp(extensions[0], "NULL") != 0){
		for(int i = 0; i < num; i++){
			if(strcmp(file_ext, extensions[i]) == 0)
				n = 1;
		}
	}
	return n;
}

int check_existing_file(DIR * dir_origin, const char * name,  const char * destiny, const char * source){
	DIR * dir_des;
	struct dirent *pDirent;
	int n = 0;
	char target[255];
	strcpy(target, destiny);
	strcpy(target, reformat_path(target));

	dir_des = opendir(destiny);
	if(dir_des == NULL){
	printf("Error: No se pudo abrir el directorio: %s\n", destiny);
	closedir(dir_origin);
	exit(1);
	}

	while((pDirent = readdir(dir_des)) != NULL){
		if((strcmp(pDirent->d_name, ".")!= 0) && (strcmp(pDirent->d_name, "..")!= 0)){
			if(strcmp(name, pDirent->d_name) == 0){
				strcat(target, pDirent->d_name);
				if(get_last_modified(source) <= get_last_modified(target))	
					n = 1;
			}
		}
	}
	closedir(dir_des);
	return n;
}

void copy_new_directory(DIR * dir_origin, const char * origin, const char * destiny, int num, unsigned size_param, int argc, char extensions[][argc]){
	struct dirent *pDirent;

	char source[255];
	char target[255];

	while((pDirent = readdir(dir_origin)) != NULL){
		if((strcmp(pDirent->d_name, ".")!= 0) && (strcmp(pDirent->d_name, "..")!= 0)){
			strcpy(source, build_file_name(origin, pDirent->d_name));
			strcpy(target, build_file_name(destiny, pDirent->d_name));

			if(pDirent-> d_type == 4){
				strcpy(source, reformat_path(source));
				strcpy(target, reformat_path(target));
				handle_directories(source, target, num, size_param, argc, extensions);
			}
			else {
				unsigned size = get_file_size(source);
				if((check_extension(get_file_extension(pDirent->d_name),num, argc, extensions) == 0) && ((size <= size_param) || (size_param == 0))){
					copy_file(source, target);
					printf("c %s\n", source);
				}
				else
					printf("x %s\n", source);
			}
		}
	}
}

void copy_existing_directory(DIR * dir_origin, const char * origin, const char * destiny, int num, unsigned size_param, int argc, char extensions[][argc]){
	struct dirent *pDirent;

	char source[255];
	char target[255];

	while((pDirent = readdir(dir_origin)) != NULL){
		if((strcmp(pDirent->d_name, ".")!= 0) && (strcmp(pDirent->d_name, "..")!= 0)){
			strcpy(source, build_file_name(origin, pDirent->d_name));
			strcpy(target, build_file_name(destiny, pDirent->d_name));

			if(pDirent-> d_type == 4){
				strcpy(source, reformat_path(source));
				strcpy(target, reformat_path(target));
				handle_directories(source, target, num, size_param, argc, extensions);
			}
			else{
				unsigned size = get_file_size(source);
				if((check_extension(get_file_extension(pDirent->d_name),num, argc, extensions) == 1) || ((size > size_param) && (size_param != 0)))
					printf("x %s\n", source);
				else if(check_existing_file(dir_origin, pDirent->d_name, destiny, source) == 1)
					printf("e %s\n", source);
				else{
					copy_file(source, target);
					printf("c %s\n", source);
				}
			}
		}	
	}
}

void handle_directories(const char * origin, const char * destiny, int num, unsigned size_param, int argc, char extensions[][argc]){
	DIR * dir_origin;
	DIR *dir_des;
	dir_origin = opendir(origin);
	int status;

	if(dir_origin == NULL){
		printf("Error: No se pudo abrir el directorio: %s\n", origin);
		exit(1);
	}

	status = mkdir(destiny, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if(status != 0){
		if(errno == EEXIST){
			
			copy_existing_directory(dir_origin, origin, destiny, num, size_param, argc, extensions);	
		}
		else{
			printf("ERROR: No se pudo crear el directorio: %s\n", destiny);
			closedir(dir_origin);
			exit(1);
		}		
	}
	else{
		copy_new_directory(dir_origin, origin, destiny, num, size_param, argc, extensions);
		closedir(dir_origin);
	}

}

int main(int argc, char const *argv[]){
	char extensions[20][argc];
	int num;
	unsigned size_param;

	if(argc < 3){
		printf("Los parametros origen y destino son obligatorios: backup <origen> <destino>\n");
		exit(1);
	}

	if (argc > 3){
		num = get_extensions(argc, argv, extensions);
		size_param = get_size_param(argc, argv);
	}
	else if(argc == 3){
		strcpy(extensions[0], "NULL");
		size_param = 0;
	}

	char directory[255];
	char dir_destiny[255];
	strcpy(directory, argv[1]);
	strcpy(directory, reformat_path(directory));
	strcpy(dir_destiny, argv[2]);
	strcpy(dir_destiny, reformat_path(dir_destiny));
	handle_directories(directory, dir_destiny, num, size_param, argc, extensions);
	printf("El respaldo fue realizado de forma exitosa.\n");
	return 0;
}