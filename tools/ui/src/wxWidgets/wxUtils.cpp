#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/dir.h>
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