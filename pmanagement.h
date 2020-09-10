#ifndef PMANAGEMENT_H
#define PMANAGEMENT_H

#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace pView{

    typedef uint32_t u32;

    class iProcess{
    private:
        PROCESSENTRY32 pe32;
    public:
        iProcess(const PROCESSENTRY32 &aux){
            pe32 = aux;
        }
        u32 getPID(){return pe32.th32ProcessID;}
        u32 getThreadcount(){return pe32.cntThreads;}
        u32 getParentPID(){return pe32.th32ParentProcessID;}
        std::string getProcessName(){
            std::wstring wstr(pe32.szExeFile);
            std::string result;
            for(char x : wstr) result += x;
            return result;
        }
        long getPrioritybase(){return pe32.pcPriClassBase;}
        u32 getPriorityclass(){
            HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
            DWORD dwPriorityClass = 0;
            if( hProcess == NULL )
                printf("OpenProcess\n");
            else{
                dwPriorityClass = GetPriorityClass( hProcess );
                if( !dwPriorityClass ) printf("GetPriorityClass\n");
                CloseHandle( hProcess );
            }
            return dwPriorityClass;
        }
    };

    class ProcessList{
    private:
        std::unordered_map<u32,iProcess*> u_plist;
        std::unordered_map<std::string,iProcess*> szExe_plist;
    public:
        ProcessList(){
            PROCESSENTRY32 pe32;
            HANDLE hProcessSnap;

            hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
            if( hProcessSnap == INVALID_HANDLE_VALUE ){
                printf("CreateToolhelp32Snapshot (of processes)");
            }

            pe32.dwSize = sizeof( PROCESSENTRY32 );

            if( !Process32First( hProcessSnap, &pe32 ) ){
                printf("Process32First\n");
                CloseHandle( hProcessSnap );
            }

            do{
                iProcess *p = new iProcess(pe32);
                u_plist[p->getPID()] = p;
                szExe_plist[p->getProcessName()] = p;
            }while(Process32Next( hProcessSnap, &pe32 ));
            CloseHandle( hProcessSnap );
        }

        ~ProcessList(){
            for(auto p:u_plist) delete p.second;
        }

        std::vector<iProcess> getProcesses(){
            std::vector<iProcess> array;
            for(auto x:u_plist){
                array.push_back(*x.second);
            }
            sort(array.begin(),array.end(),[](iProcess &p1,iProcess &p2){
                return p1.getPID() < p2.getPID();
            });
            return array;
        }
        iProcess operator[](const u32& i){return *u_plist[i];}
        iProcess operator[](const std::string& str){return *szExe_plist[str];}

    };

    class iModule{
    private:
        MODULEENTRY32 me32;
    public:
        iModule(const MODULEENTRY32 &aux){
            me32 = aux;
        }

        std::string getModuleName(){
            std::wstring wstr(me32.szModule);
            std::string result;
            for(char x : wstr) result += x;
            return result;
        }
        std::string getExecutable(){
            std::wstring wstr(me32.szExePath);
            std::string result;
            for(char x : wstr) result += x;
            return result;
        }
        u32 getPID(){return me32.th32ProcessID;}
        u32 getRefcountg(){return me32.GlblcntUsage;}
        u32 getRefcountp(){return me32.ProccntUsage;}
        u32 getBaseAddress(){return (DWORD)(me32.modBaseAddr);}
        u32 getBaseSize(){return me32.modBaseSize;}
    };

    class ModuleList{
    private:
        std::unordered_map<std::string,iModule*> szModule_mlist;
    public:
        ModuleList(DWORD dwPID){
            HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
            MODULEENTRY32 me32;

            hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
            if( hModuleSnap == INVALID_HANDLE_VALUE ){
                printf("CreateToolhelp32Snapshot (of modules)\n");
            }

            me32.dwSize = sizeof( MODULEENTRY32 );

            if( !Module32First( hModuleSnap, &me32 ) ){
                printf("Module32First\n");
                CloseHandle( hModuleSnap );
            }
            do{
                iModule *m = new iModule(me32);
                szModule_mlist[m->getModuleName()] = m;
            }while(Module32Next( hModuleSnap, &me32 ));
            CloseHandle( hModuleSnap );
        }
        ~ModuleList(){
            for(auto m:szModule_mlist) delete m.second;
        }

        std::vector<iModule> getModules(){
            std::vector<iModule> array;
            for(auto x:szModule_mlist){
                array.push_back(*x.second);
            }
            return array;
        }

        iModule operator[](const std::string& str){return *szModule_mlist[str];}

    };

}

namespace pManagement{

    typedef uint64_t u64;
    typedef uint32_t u32;
    typedef uint16_t u16;
    typedef uint8_t u8;

    #define WRITABLE (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

    struct Address{
        u32 address;
    private:
        HANDLE *m_hProcess;
    public:

        Address(HANDLE *h,u32 addr):address(addr),m_hProcess(h){}

        uint64_t getValue(uint8_t nBytes){
            uint8_t Buffer[8] = {0};
        	ReadProcessMemory(*m_hProcess, (PBYTE*)address, Buffer, nBytes, 0);
        	switch(nBytes){
        	case 1: return *(uint8_t*)Buffer;
        	case 2: return *(uint16_t*)Buffer;
        	case 4: return *(uint32_t*)Buffer;
        	case 8: return *(uint64_t*)Buffer;
        	default: return 0;
        	}
        }

        template<typename T>
        void setValue(const T &value){
        	WriteProcessMemory(*m_hProcess, (LPVOID)address, &value, sizeof(value),0);
        }

    };

    class pHandle{
    private:
        HANDLE m_hProcess;
        DWORD PID;
        std::vector<MEMORY_BASIC_INFORMATION> mbi_list;
        SYSTEM_INFO si;

        struct compareMBI_address{
	       u32 value( const MEMORY_BASIC_INFORMATION& mbi ) const{
	          return (u32)mbi.BaseAddress + mbi.RegionSize;
	       }
	       u32 value( u32 address ) const{
	          return address;
	       }
	       template< typename T1, typename T2 >
	       bool operator()( T1 const& t1, T2 const& t2 ) const{
	           return value(t1) < value(t2);
	       }
	    };

    public:

        pHandle(const DWORD &PID){
            this->PID = PID;
            m_hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, PID);
            GetSystemInfo(&si);
            walk();
        }
        ~pHandle(){
            CloseHandle(m_hProcess);
        }

        void walk(){
            MEMORY_BASIC_INFORMATION mbi;
            LPVOID lpMem = 0;
            mbi_list.clear();
            while(lpMem < si.lpMaximumApplicationAddress){
                VirtualQueryEx(m_hProcess, lpMem, &mbi, sizeof(mbi));
                if((mbi.State&MEM_COMMIT) && (mbi.Protect&WRITABLE)) mbi_list.push_back(mbi);
                lpMem = (LPVOID)( (DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize );
            }
        }

        u32 ReadMemory(const u32 address, u8 *Buffer, const u32 nBytes){
            u32 BytesRead=0;
            ReadProcessMemory(m_hProcess, (PBYTE*)address, Buffer, nBytes, (PDWORD)&BytesRead);
            return BytesRead;
        }
        u32 WriteMemory(const u32 address, u8 *Buffer, const u32 nBytes){
            u32 BytesRead=0;
            WriteProcessMemory(m_hProcess, (LPVOID)address, Buffer, nBytes, (PDWORD)&BytesRead);
            return BytesRead;
        }
        MEMORY_BASIC_INFORMATION search_MBIperBaseAddress(const u32 &address){
            return *std::lower_bound(mbi_list.begin(), mbi_list.end(), address, compareMBI_address());
        }

        u32 getPID(){return PID;}

        HANDLE getHandle(){return m_hProcess;}

        template< typename T>
        std::vector< Address > searchValue(const T &value, u8 nBytes){
            std::vector< Address > addressList;
            for(auto mbi:mbi_list){
                u32 regionCount = (u32)mbi.BaseAddress;
                u8 *Buffer = new u8[(u32)mbi.RegionSize];
                ReadMemory((u32)mbi.BaseAddress,Buffer,(u32)mbi.RegionSize);
                while(regionCount < (u32)mbi.BaseAddress+mbi.RegionSize){
                    if(memcmp(&Buffer[ regionCount - (u32)mbi.BaseAddress ], &value, nBytes) == 0){
                        addressList.push_back( Address(&m_hProcess,regionCount) );
                    }
                    regionCount += nBytes;
                }
                delete[] Buffer;
            }
            return addressList;
        }

        void printRegionMemory(const MEMORY_BASIC_INFORMATION &mbi){
            u32 regionCount = (u32)mbi.BaseAddress&0xFFFFFFF0;
            u8 *Buffer = new u8[(u32)mbi.RegionSize];
            ReadMemory((u32)mbi.BaseAddress,Buffer,sizeof(Buffer));
            while(regionCount < (u32)mbi.BaseAddress+mbi.RegionSize){
                printf("[0x%08X] ",regionCount&0xFFFFFFF0);
                for(u8 i=0;i<0x10;i++)
                    if(regionCount < (u32)mbi.BaseAddress || regionCount >= (u32)mbi.BaseAddress + mbi.RegionSize)
                        printf("?? ");
                    else{
                        printf("%02X ",Buffer[ regionCount - (u32)mbi.BaseAddress ]);
                        regionCount++;
                    }
                printf("\n");
            }
            delete[] Buffer;
        }

    };

}

#endif // PMANAGEMENT_H
