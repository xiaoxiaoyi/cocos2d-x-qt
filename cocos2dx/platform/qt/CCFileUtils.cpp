/****************************************************************************
Copyright (c) 2012 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCCommon.h"
#include "ccMacros.h"

#include "CCFileUtils.h"
#include "CCString.h"
#define __CC_PLATFORM_FILEUTILS_CPP__
#include "platform/CCFileUtilsCommon_cpp.h"

using namespace std;

NS_CC_BEGIN

static CCFileUtils* s_pFileUtils = NULL;
static string s_strResourcePath = "";
static std::map<std::string, std::string> s_fullPathCache;

//void CCFileUtils::setResourcePath(const char* pszResourcePath) {
//    CC_UNUSED_PARAM(pszResourcePath);
//    CCAssert(0, "Have not implement!");
//}
CCFileUtils* CCFileUtils::sharedFileUtils(void) {
    if ( s_pFileUtils == NULL ) {
        s_pFileUtils = new CCFileUtils();
        s_pFileUtils->init();
    }
    return s_pFileUtils;
}

bool CCFileUtils::init(void)
{
    m_strDefaultResRootPath = "";
    m_searchPathArray.push_back(m_strDefaultResRootPath);
    m_searchResolutionsOrderArray.push_back("");

    return true;
}

void CCFileUtils::purgeFileUtils()
{
    if (s_pFileUtils != NULL)
    {
        s_pFileUtils->purgeCachedEntries();
        CC_SAFE_DELETE(s_pFileUtils->m_pFilenameLookupDict);
    }

    CC_SAFE_DELETE(s_pFileUtils);
}

void CCFileUtils::purgeCachedEntries()
{
    s_fullPathCache.clear();
    return;
}

const char* CCFileUtils::fullPathFromRelativePath(const char *pszRelativePath) {
	CCString *pRet = new CCString();
	pRet->autorelease();
	pRet->m_sString = s_strResourcePath + pszRelativePath;
	return pRet->m_sString.c_str();
}

const char *CCFileUtils::fullPathFromRelativeFile(const char *pszFilename, const char *pszRelativeFile) {
	std::string relativeFile = pszRelativeFile;
	CCString *pRet = new CCString();
	pRet->autorelease();
	pRet->m_sString = relativeFile.substr(0, relativeFile.rfind('/')+1);
	pRet->m_sString += pszFilename;
	return pRet->m_sString.c_str();
}

std::string CCFileUtils::getPathForFilename(const std::string& filename, const std::string& resourceDirectory, const std::string& searchPath)
{
    std::string file = filename;
    std::string file_path = "";
    size_t pos = filename.find_last_of("/");
    if (pos != std::string::npos)
    {
        file_path = filename.substr(0, pos+1);
        file = filename.substr(pos+1);
    }

    // searchPath + file_path + resourceDirectory
    std::string path = searchPath;
    if (path.size() > 0 && path[path.length()-1] != '/')
    {
        path += "/";
    }
    path += file_path;
    path += resourceDirectory;

    if (path.size() > 0 && path[path.length()-1] != '/')
    {
        path += "/";
    }
    path += file;

    return path;
}

std::string CCFileUtils::fullPathForFilename(const char* pszFileName)
{
    CCAssert(pszFileName != NULL, "CCFileUtils: Invalid path");

    std::string relPath(pszFileName);

//    // Return directly if it's an absolute path.
//    if ([relPath isAbsolutePath]) {
//        return pszFileName;
//    }

    // Already Cached ?
    std::map<std::string, std::string>::iterator cacheIter = s_fullPathCache.find(pszFileName);
    if (cacheIter != s_fullPathCache.end()) {
        //CCLOG("Return full path from cache: %s", cacheIter->second.c_str());
        return cacheIter->second;
    }

    std::string fullpath = "";

    // in Lookup Filename dictionary ?
    std::string newfilename = this->getNewFilename(pszFileName);

    for (std::vector<std::string>::iterator searchPathsIter = m_searchPathArray.begin();
         searchPathsIter != m_searchPathArray.end(); ++searchPathsIter) {
        for (std::vector<std::string>::iterator resOrderIter = m_searchResolutionsOrderArray.begin();
             resOrderIter != m_searchResolutionsOrderArray.end(); ++resOrderIter) {

            fullpath = this->getPathForFilename(newfilename, *resOrderIter, *searchPathsIter);

            if (fullpath.length() > 0)
            {
                // Adding the full path to cache if the file was found.
                s_fullPathCache.insert(std::pair<std::string, std::string>(pszFileName, fullpath));
                return fullpath;
            }
        }
    }

    // The file wasn't found, return the file name passed in.
    return pszFileName;
}

unsigned char* CCFileUtils::getFileData(const char* pszFileName, const char* pszMode, unsigned long * pSize) {

	string fullPath = pszFileName;
	unsigned char * pData = 0;

	do
	{
		// read rrom other path than user set it
		FILE *fp = fopen(pszFileName, pszMode);
		CC_BREAK_IF(!fp);

		fseek(fp,0,SEEK_END);
		*pSize = ftell(fp);
		fseek(fp,0,SEEK_SET);
		pData = new unsigned char[*pSize];
		*pSize = fread(pData,sizeof(unsigned char), *pSize,fp);
		fclose(fp);
	}while (0);

#if 1
    if ( !pData ) {
        printf("Get data from file(%s) failed!", fullPath.c_str());
    }
#else
    if (! pData && getIsPopupNotify())
    {
        std::string title = "Notification";
        std::string msg = "Get data from file(";
        msg.append(fullPath.c_str()).append(") failed!");
        CCMessageBox(msg.c_str(), title.c_str());
    }
#endif
	return pData;

}

//void CCFileUtils::setResource(const char* pszZipFileName) {
//    CC_UNUSED_PARAM(pszZipFileName);
//	CCAssert(0, "Have not implement!");
//}

//int CCFileUtils::ccLoadFileIntoMemory(const char *filename, unsigned char **out) {
//    CC_UNUSED_PARAM(filename);
//    CC_UNUSED_PARAM(out);

//	CCAssert(0, "Have not implement!");
//	return 0;
//}

string CCFileUtils::getWriteablePath() {
	//return current resource path
	return s_strResourcePath;
}

NS_CC_END
