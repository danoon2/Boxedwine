#ifndef __WXUTILS_H__
#define __WXUTILS_H__

wxArrayString wxUtilRemoveDuplicates(const wxArrayString& values);
bool wxCopyDir(wxString sFrom, wxString sTo);
bool wxExtractZipFile(const wxString& aZipFile, const wxString& aTargetDir, const wxString& fileToExtract="");

#ifdef _WINDOWS
class BoxedContainer;
void updateWindowsIntegrationRegistry(BoxedContainer* container);
void deleteWindowsIntegrationRegistry();
#endif

wxString getContainerNameAssociatedWithIntegration();

#endif