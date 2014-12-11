#include "stdafx.h"
#include "FileProcess.h"
#include <io.h>
//#include <direct.h>

BOOL IsExistFile(CString Filename)
{
	struct _finddata_t  c_file;    
	long hFile;

	if( (hFile = _findfirst(Filename, &c_file )) != -1L )
		return TRUE;
	else
		return FALSE;
}
BOOL FindNextFile_L(CString &Filename)
{
	int count1 = Filename.ReverseFind('\\');
	//TRACE("The Filename is %s\n",Filename);
	CString Path = Filename.Left(count1+1);
	CString FileExtname = Path + "*" + Filename.Right(4);
	//TRACE("The FileExtname is %s\n",FileExtname);
	CString LastFilename ;

	struct _finddata_t c_file;    
	long hFile;
	BOOL OK = FALSE;
	 
	if( (hFile = _findfirst(FileExtname, &c_file )) != -1L )
	{
		LastFilename = Path + c_file.name;	
		//TRACE("next image filename is %s\n",LastFilename);
		while( _findnext( hFile, &c_file ) == 0 )            
		{
			if(LastFilename == Filename) OK=TRUE;
			if(OK)
			{
				Filename = Path + c_file.name;
				return TRUE;
			}
			LastFilename = Path + c_file.name;			
		}		 
	}
	_findclose( hFile );
	return FALSE;
}

BOOL FindPrevFile_L(CString &Filename)
{
	int count1 = Filename.ReverseFind('\\');
	//TRACE("The Filename is %s\n",Filename);
	CString Path = Filename.Left(count1+1);
	CString FileExtname = Path + "*" + Filename.Right(4);
	//TRACE("The FileExtname is %s\n",FileExtname);
	CString LastFilename ;

	struct _finddata_t c_file;    
	long hFile;
	BOOL OK = FALSE;
	 
	if( (hFile = _findfirst(FileExtname, &c_file )) != -1L )
	{
		LastFilename = Path + c_file.name;	
		//TRACE("next image filename is %s\n",LastFilename);
		while( _findnext( hFile, &c_file ) == 0 )            
		{
			CString Nowfile = Path + c_file.name;
			if(Nowfile == Filename) OK=TRUE;
			if(OK)
			{
				Filename = LastFilename;
				return TRUE;
			}
			LastFilename = Nowfile;			
		}		 
	}
	_findclose( hFile );  
	return FALSE;
}
int GetFileNumber(CString FileName)
{
	int nRet = 0;
	int count = FileName.GetLength();
	int StartPos = count-4;
	if(StartPos< 1 || FileName.GetAt(StartPos)!='.')
	{
		AfxMessageBox("文件名格式有误\n非*.*形式!");
		return -1;
	}
	BOOL   ok = FALSE;
	BOOL   NumFlag = FALSE;
	BYTE tChar;
	int nPow = 1;
    while(!ok)
	{
		StartPos--;
		if(StartPos<=0) 
			ok = TRUE;
		tChar = FileName.GetAt(StartPos) - '0';
		if(tChar>=0 && tChar<= 9)
		{
			NumFlag = TRUE;
			nRet = nRet + tChar * nPow;
			nPow *= 10;
		}
		else
		{
			ok = TRUE;
		}
	}	
	if(NumFlag)
		return nRet;
	else 
		return 0;
}
CString   GetNextFileName(CString strFileCurrent)
{
	int  nCurrentFileNumber =  GetFileNumber(strFileCurrent);
//	if(nCurrentFileNumber< 0)
//		return "";
	nCurrentFileNumber += 1; 

	CString strFileNext = strFileCurrent;
	ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	
	if(!IsExistFile(strFileNext))  // 序列少一帧
	{
		nCurrentFileNumber += 1; 
		ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	}
	if(!IsExistFile(strFileNext))  // 序列少二帧
	{
		nCurrentFileNumber += 1; 
		ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	}
	return strFileNext;  
}

CString   GetPreFileName(CString strFileCurrent)
{
	int  nCurrentFileNumber =  GetFileNumber(strFileCurrent);
	//if(nCurrentFileNumber<= 0)
	//	return "";
	nCurrentFileNumber -= 1; 

	CString strFileNext = strFileCurrent;
	ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	
	if(!IsExistFile(strFileNext))  // 序列少一帧
	{
		nCurrentFileNumber -= 1; 
		ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	}
	if(!IsExistFile(strFileNext))  // 序列少二帧
	{
		nCurrentFileNumber -= 1; 
		ResetCurrentFileNameFromNumber( strFileNext , nCurrentFileNumber);
	}
	return strFileNext;  
}

/////////////////////////////////////////////////////////
//////函数用途：重置文件名
//////Author  : 朱宪伟 引用SPD-3
//////Modifier: 李壮   
//////Date    
////////////////////////////////////////////////////////
void ResetCurrentFileNameFromNumber( CString & FileName , int FileNum)
{
	int count = FileName.GetLength();
	int StartPos = count - 4;
	if(FileName.GetAt(StartPos)!='.')
	{
		AfxMessageBox("文件名格式有误\n非*.*形式!");
		return ;
	}
	BYTE tChar,n;
	int nPow = 1;
	if (FileNum < 0)
	{
		return;
	}
    while(1)
	{
		if((FileNum+1) / nPow == 0)
			return;
		n = (FileNum % (nPow * 10)) / nPow; 
		StartPos--;
		if(StartPos<=0) 
			return;
		tChar = FileName.GetAt(StartPos) - '0';
		if(tChar>=0 && tChar<= 9)
		{
			
			FileName.SetAt(StartPos, n + '0');
			nPow *= 10;
		}
		else
		{
			FileName.Insert(StartPos+1, n + '0');
			nPow *= 10;
		}
	}	
}

void GetFolderPathFromFilePath( CString FileName , CString &FilePath)
{
	FilePath= FileName;
	int count = FileName.GetLength();
	int StartPos = count;
	BYTE tChar;
	int nPow = 1;

	while(1)
	{		
		StartPos--;
		if(StartPos<=0) 
			return;
		tChar = FileName.GetAt(StartPos);
		if(tChar=='\\')
		{
			FilePath= FileName.Left(StartPos+1);
			return;
		}
	}	
}

void GetFileNameFromFilePath( CString FileName , CString &fname)
{
	fname= FileName;
	int count = FileName.GetLength();
	int StartPos = count;
	BYTE tChar;
	int nPow = 1;

	while(1)
	{		
		StartPos--;
		if(StartPos<=0) 
			return;
		tChar = FileName.GetAt(StartPos);
		if(tChar=='\\')
		{
			fname= FileName.Right(FileName.GetLength()-StartPos-1);
			return;
		}
	}	
}