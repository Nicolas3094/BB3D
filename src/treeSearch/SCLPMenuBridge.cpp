#include "SCLPMenuBridge.hpp"
#include "../readData/ExperimentResults.hpp"
#include <fstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
namespace {
std::wstring wide(const std::string &text) {
    const auto size=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0);
    if(!size)throw std::runtime_error("Ruta SCLP no valida en UTF-8.");
    std::wstring result(size,L' ');
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),&result[0],size);return result;
}
// Windows argv quoting; CreateProcess bypasses shell expansion entirely.
std::wstring argument(const std::string &text) {
    std::wstring result=L"\"";size_t slashes=0;
    for(wchar_t ch:wide(text)) {
        if(ch==L'\\'){++slashes;continue;}
        result.append(ch==L'"'?slashes*2+1:slashes,L'\\');slashes=0;result+=ch;
    }
    result.append(slashes*2,L'\\');return result+L'"';
}
}
#endif
std::string executeSCLPMenu(const BRMenuSelection &selection) {
#ifndef _WIN32
    (void)selection;
    throw std::runtime_error("El lanzador de menu SCLP requiere Windows/PowerShell; usa el CLI independiente en otras plataformas.");
#else
    const auto &o=selection.options;
    if(!std::ifstream(o.root+"/build/sclp-release/sclp_reference.exe"))
        throw std::runtime_error("Falta compilar SCLP: ejecuta ./src/treeSearch/Build-SCLP.ps1");
    const auto script=o.root+"/src/treeSearch/Run-SCLP-Menu.ps1";
    if(!std::ifstream(script))throw std::runtime_error("No se encontro el lanzador SCLP: "+script);
    const auto directory=createResultSessionDirectory(o.output.empty()?o.root+"/Results/BR":o.output,selection.sessionName);
    wchar_t windowsDirectory[MAX_PATH];
    const auto length=GetWindowsDirectoryW(windowsDirectory,MAX_PATH);
    if(!length || length>=MAX_PATH)throw std::runtime_error("No se encontro la carpeta de Windows.");
    const std::wstring powershell=std::wstring(windowsDirectory)+L"\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    std::wstring command=L"\""+powershell+L"\" -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "+argument(script)
        +L" -DataRoot "+argument(o.root)+L" -Dataset "+argument(o.dataset)
        +L" -FirstInstance "+std::to_wstring(o.firstInstance)+L" -Count "+std::to_wstring(o.instances)
        +L" -Rounds "+std::to_wstring(o.rounds)+L" -OutputDirectory "+argument(directory);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    std::cout<<std::flush;
    if(!CreateProcessW(powershell.c_str(),&command[0],nullptr,nullptr,TRUE,0,nullptr,nullptr,&startup,&process)) {
        const auto message="No se pudo iniciar Windows PowerShell para SCLP. Error Windows: "+std::to_string(GetLastError());
        saveResultText(directory+"/resumen.md",message+"\n");
        throw std::runtime_error(message+"; sesion: "+directory);
    }
    CloseHandle(process.hThread);WaitForSingleObject(process.hProcess,INFINITE);
    DWORD status=1;GetExitCodeProcess(process.hProcess,&status);CloseHandle(process.hProcess);
    if(status!=0)throw std::runtime_error("SCLP no completo la sesion. Revisa "+directory+"/resumen.md");
    return directory;
#endif
}
