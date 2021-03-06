#include "SharedMemory.hpp"

void SharedMemory::LogError(const char *logPtr)
{
#ifdef SHAREDMEM_LOGGING
	if (this->logFilePtr == NULL)
	{
		this->logFilePtr = fopen(SHAREDMEM_FILENAME, "a");
	}
	if (this->logFilePtr != NULL)
	{
		fprintf(this->logFilePtr, "%s\r\n", logPtr);
		fprintf(this->logFilePtr, "Windows Error code: %d\r\n\r\n", GetLastError());
	}
#endif
}

SharedMemory::SharedMemory(LPCWSTR newNamePtr, unsigned int size)
{
    this->mapsize = size;
    this->namePtr = newNamePtr;
	this->isSharedMemoryHooked = false;
#ifdef SHAREDMEM_LOGGING
	this->logFilePtr = NULL;
#endif

    hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE, // use paging file
            NULL, // default security
            PAGE_READWRITE, // read/write access
            0, // maximum object size (high-order DWORD)
            size, // maximum object size (low-order DWORD)
            this->namePtr); // name of mapping object
	LogError("Created file map");
    if (hMapFile == NULL)
    {
		LogError("but it's NULL!");
        if(GetLastError() == (DWORD)183) // already exists
        {
                hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, this->namePtr);
                if (hMapFile == NULL)
                {
                        LogError("Could not open existing file mapping");
                        return;
                }
        }
        else
        {
                LogError("Could not create file mapping object");
                return;
        }
    }
	else
	{
		LogError("and it's not NULL!");
	}

    this->pBufferPtr = (void*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (this->pBufferPtr == NULL)
    {
        LogError("Could not reserve buffer for shared memory");
        CloseHandle(hMapFile);
    }
	else
	{
		memset(this->pBufferPtr, 0, size);
		this->isSharedMemoryHooked = true;
		LogError("Opened MMF");
	}

}


void SharedMemory::Close(void)
{
#ifdef SHAREDMEM_LOGGING
		if (logFilePtr != NULL)
		{
			fclose(logFilePtr);
			// TODO: Is this closed properly?
		}
#endif
        if (isSharedMemoryHooked)
        {
                if (pBufferPtr != NULL) UnmapViewOfFile(pBufferPtr);
                if (hMapFile != NULL) CloseHandle(hMapFile);
        }

        isSharedMemoryHooked = false;

}
