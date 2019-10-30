#ifndef __ResourceSortingFunctions_h__
#define __ResourceSortingFunctions_h__

class Resource;

bool sortByUIDAscending(const Resource* a, const Resource* b);
bool sortByUIDDescending(const Resource* a, const Resource* b);

bool sortByNameAscending(const Resource* a, const Resource* b);
bool sortByNameDescending(const Resource* a, const Resource* b);

bool sortByFileAscending(const Resource* a, const Resource* b);
bool sortByFileDescending(const Resource* a, const Resource* b);

bool sortByExportedFileAscending(const Resource* a, const Resource* b);
bool sortByExportedFileDescending(const Resource* a, const Resource* b);

bool sortByReferenceAscending(const Resource* a, const Resource* b);
bool sortByReferenceDescending(const Resource* a, const Resource* b);

bool sortByTypeAscending(const Resource* a, const Resource* b);
bool sortByTypeDescending(const Resource* a, const Resource* b);

#endif __ResourceSortingFunctions_h__