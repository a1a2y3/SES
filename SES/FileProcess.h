#if !defined  _IMGAEPROC_567864_H
#define      _IMGAEPROC_567864_H

BOOL IsExistFile(CString Filename);
//����ͼ���� �����ļ����Ĵ�����
BOOL FindNextFile_L(CString &Filename);
BOOL FindPrevFile_L(CString &Filename);
int       GetFileNumber(CString FileName);
void      ResetCurrentFileNameFromNumber( CString & FileName , int FileNum);
CString   GetNextFileName(CString strCurrentFileName);
CString   GetPreFileName(CString strCurrentFileName);
void GetFolderPathFromFilePath( CString FileName , CString &FilePath);
void GetFileNameFromFilePath( CString FileName , CString &fname);

#endif
