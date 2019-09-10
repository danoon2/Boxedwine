#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filesys.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#include "wxUtils.h"

wxArrayString wxUtilRemoveDuplicates(const wxArrayString& values) {
    wxArrayString results; // new empty wxArrayString
    for(int i=0; i<values.GetCount(); i++)
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