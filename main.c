

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>

int main(int argc, char **argv)
{
	char tbuf[512];
	WIN32_STREAM_ID sid;
	ZeroMemory(&sid, sizeof(WIN32_STREAM_ID));
	DWORD dw1,dw2,dwRead;
	int numofstreams = 0;
	//Буфер для имени потока в формате Unicode
	WCHAR wszStreamName[MAX_PATH];
	LPVOID lpContext = NULL;
	/*
	 *        Открываем файл для чтения с параметром
	 *        FILE_FLAG_BACKUP_SEMANTICS, что позволяет нам
	 *        открывать не только файлы, но и каталоги с дисками.
	 */
	if(argc!=2) return puts("Invalid number of parameters!"),0;
	HANDLE hFile=CreateFile(argv[1],GENERIC_READ,FILE_SHARE_READ,
			NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		exit(printf("\nError: Could't open file, directory or disk %s\n",
				argv[1])&0);
	DWORD dwStreamHeaderSize =
			  (LPBYTE)&sid.cStreamName
			- (LPBYTE)&sid + sid.dwStreamNameSize;
	while(BackupRead(hFile,(LPBYTE)&sid,dwStreamHeaderSize,
			&dwRead,FALSE,TRUE,&lpContext))
	{
		//Если тип потока неверный, значит прерываем цикл
		if (sid.dwStreamId == BACKUP_INVALID) break;
		ZeroMemory(&wszStreamName,sizeof(wszStreamName));
		//Получаем имя потока
		if(!BackupRead(hFile,(LPBYTE)wszStreamName,
				sid.dwStreamNameSize,&dwRead,FALSE,TRUE,&lpContext))
			break;
		if(	  sid.dwStreamId==BACKUP_DATA
			||sid.dwStreamId==BACKUP_ALTERNATE_DATA)
		{
			numofstreams++;
			switch (sid.dwStreamId)
			{
				case BACKUP_DATA:
					printf("%8u\t%s::$DATA\n",sid.Size.u.LowPart,argv[1]);
					break;
				case BACKUP_ALTERNATE_DATA:
					CharToOemW(wszStreamName, tbuf);
					printf("%8u\t%s%s\n",sid.Size.u.LowPart,argv[1],tbuf);
					break;
			}
		}
		//Перемещаемся к следующему потоку
		BackupSeek(hFile,sid.Size.LowPart,sid.Size.HighPart,
				&dw1,&dw2,&lpContext);
		//Очищаем структуру перед следующим проходом цикла
		ZeroMemory(&sid,sizeof(sid));
	}
	//Очищаем lpContext, содержащий служебную информацию
	//для работы функции BackupRead
	BackupRead(hFile,NULL,0,&dwRead,TRUE,FALSE,&lpContext);
	CloseHandle(hFile);
	return 0;
}
