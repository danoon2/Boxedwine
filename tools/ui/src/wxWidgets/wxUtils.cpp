#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/dir.h"
#include "wx/filesys.h"
#include "wx/zipstrm.h"
#include "wx/wfstream.h"
#include "wx/config.h"
#include "wx/filename.h"

#include "wxUtils.h"
#include "GlobalSettings.h"
#include "BoxedContainer.h"

wxArrayString wxUtilRemoveDuplicates(const wxArrayString& values) {
    wxArrayString results; // new empty wxArrayString
    for(int i=0; i<(int)values.GetCount(); i++)
    {
        const wxString &s = values[i];
        if( results.Index(s) == wxNOT_FOUND )
            results.Add(s);
    }
    return results;
}

bool wxCopyDir(wxString sFrom, wxString sTo)
{
    if (sFrom[sFrom.Len() - 1] != '\\' && sFrom[sFrom.Len() - 1] != '/') sFrom += wxFILE_SEP_PATH;
    if (sTo[sTo.Len() - 1] != '\\' && sTo[sTo.Len() - 1] != '/') sTo += wxFILE_SEP_PATH;

    if (!::wxDirExists(sFrom)) {
        return false;
    }
    if (!wxDirExists(sTo)) {
        if (!wxMkdir(sTo)) {
            return false;
        }
    }

    wxDir fDir(sFrom);
    wxString sNext = wxEmptyString;
    bool bIsFile = fDir.GetFirst(&sNext);
    while (bIsFile) {
        const wxString sFileFrom = sFrom + sNext;
        const wxString sFileTo = sTo + sNext;
        if (::wxDirExists(sFileFrom)) {
            wxCopyDir(sFileFrom, sFileTo);
        }
        else {
            if (!::wxFileExists(sFileTo)) {
                if (!::wxCopyFile(sFileFrom, sFileTo)) {
                    return false;
                }
            }
        }
        bIsFile = fDir.GetNext(&sNext);
    }
    return true;
}

bool wxExtractZipFile(const wxString& aZipFile, const wxString& aTargetDir, const wxString& fileToExtract) {
    bool ret = true;

    //wxFileSystem fs;
    std::auto_ptr<wxZipEntry> entry(new wxZipEntry());
    wxString fileToExtractDirectory;

    if (fileToExtract.Length()) {
        fileToExtractDirectory = wxPathOnly(fileToExtract)+wxFileName::GetPathSeparator();
    }
    do {  
        wxFileInputStream in(aZipFile);

        if (!in) {
            wxLogError(_T("Can not open file '")+aZipFile+_T("'."));
            ret = false;
            break;
        }
        wxZipInputStream zip(in);

        while (entry.reset(zip.GetNextEntry()), entry.get() != NULL) {
            // access meta-data
            wxString name = entry->GetName();
            name = aTargetDir + wxFileName::GetPathSeparator() + name;

            // read 'zip' to access the entry's data
            if (entry->IsDir()) {
                if (fileToExtract.Length()==0 || fileToExtractDirectory.StartsWith(name)) {
                    int perm = entry->GetMode();
                    wxFileName::Mkdir(name, perm, wxPATH_MKDIR_FULL);
                }
            } else {
                if (fileToExtract.Length()==0 || name.CmpNoCase(fileToExtract)==0) {
                    zip.OpenEntry(*entry.get());
                    if (!zip.CanRead()) {
                        wxLogError(_T("Can not read zip entry '") + entry->GetName() + _T("'."));
                        ret = false;
                        break;
                    }

                    wxFileOutputStream file(name);

                    if (!file) {
                        wxLogError(_T("Can not create file '")+name+_T("'."));
                        ret = false;
                        break;
                    }
                    zip.Read(file);
                    if (fileToExtract.Length()) {
                        break;
                    }
                }
            }
        }
    } while(false);

    return ret;
}

#ifdef _WINDOWS
void startRegedit(const wxString& filepath) {
    wxString params = "/s "+filepath;
    SHELLEXECUTEINFO shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = _T("runas");                // Operation to perform
    shExInfo.lpFile = _T("regedit");       // Application to start    
    shExInfo.lpParameters = params.c_str();                  // Additional parameters
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = 0;  

    if (ShellExecuteEx(&shExInfo))
    {
        WaitForSingleObject(shExInfo.hProcess, INFINITE);
        CloseHandle(shExInfo.hProcess);
    }
}

wxString createRegistryEntry(BoxedContainer* container) {
    wxString zip = "FileSystems\\"+GlobalSettings::GetFileFromWineName(container->GetWineVersion());
    wxString root = "Containers\\"+container->GetName()+"\\root";
    wxString text = "Windows Registry Editor Version 5.00\r\n\r\n";

    zip.Replace("\\", "\\\\");
    root.Replace("\\", "\\\\");

    text+="[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NtVdm64\\0BOXEDWINE]\r\n";
    text+="\"CommandLine\"=\"-root \\\""+root+"\\\" -zip \\\""+zip+"\\\" \\\"%m\\\"\"\r\n";
    text+="\"InternalName\"=\"*\"\r\n";
    text+="\"ProductName\"=\"*\"\r\n";
    text+="\"ProductVersion\"=\"*\"\r\n";
    wxString app = GlobalSettings::exeFileLocation;
    app.Replace("\\", "\\\\");
    text+="\"MappedExeName\"=\""+app+"\"";
    return text;
}

static const char* regKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NtVdm64\\0BOXEDWINE";
wxString removeRegistryEntry() {
    wxString text = "Windows Registry Editor Version 5.00\r\n\r\n";

    text+="[-HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NtVdm64\\0BOXEDWINE]";
    return text;
}

void runRegedit(const wxString& text) {    
    wxString fileName = wxFileName::CreateTempFileName("box");
    if (!fileName.Length()) {
        return;
    }
    fileName+=".reg";
    wxFile f;
    f.Open(fileName, wxFile::write);    
    f.Write(text);
    f.Close();

    startRegedit(fileName);
}

void updateWindowsIntegrationRegistry(BoxedContainer* container) {
    wxRegKey registry(regKey);
    if (!registry.Exists()) {
        if (!registry.Create()) {
            runRegedit(createRegistryEntry(container));
            return;
        }
    }
    wxString zip = "FileSystems\\"+GlobalSettings::GetFileFromWineName(container->GetWineVersion());
    wxString root = "Containers\\"+container->GetName()+"\\root";

    if (!registry.SetValue("CommandLine", "-root \""+root+"\" -zip \""+ zip + "\" \"%m\"") ||
        !registry.SetValue("InternalName", "*") ||
        !registry.SetValue("ProductName", "*") ||
        !registry.SetValue("ProductVersion", "*") ||
        !registry.SetValue("MappedExeName", GlobalSettings::exeFileLocation)) {
        runRegedit(createRegistryEntry(container));
    }
}

void deleteWindowsIntegrationRegistry() {
    wxRegKey registry(regKey);
    if (registry.Exists()) {
        if (!registry.DeleteSelf()) {
            runRegedit(removeRegistryEntry());
        } else {
            wxRegKey registry2(regKey);
            if (registry.Exists()) {
                runRegedit(removeRegistryEntry());
            }
        }
    }
}

wxString getContainerNameAssociatedWithIntegration() {
    wxRegKey registry(regKey);
    if (registry.Exists()) {
        long index=0;
        wxString value;
        if (registry.QueryValue("CommandLine", value)) {
            if (value.StartsWith("-root \"Containers\\")) {
                value = value.SubString(18, value.Length());
                int pos = value.Find("\\");
                if (pos>=0) {
                    value = value.SubString(0, pos-1);
                    return value;
                }
            }
        }
    }
    return "";
}

#else
wxString getContainerNameAssociatedWithIntegration() {
    return "";
}

#endif

