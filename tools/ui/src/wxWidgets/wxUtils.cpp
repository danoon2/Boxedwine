#include <wx/wxprec.h>
#include <wx/wx.h>
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