# HDF5 Index and Request Processing #

## Motivation ##

Large and very hierarchical HDF5 files are often expensive to traverse.
To process a request, every (or a large subset of) group and dataset must
be visited and the attribute compared to that listed in the request.
Most often (for large files), this comparison evaluates to false.
This is what makes file (meta-) operations expensive in analysis.

This project aims at visiting every group / dataset only once:
Attributes are extracted and saved to a database. After indexing,
a request can be processed in constant time and returns a full path
to the requested dataset without even opening the hdf5 file.

## Requests ##

A request is a list of attributes with certain values. More than one
dataset can match a request. Datasets can have attributes that are not 
part of the request.

In other words: A dataset matching a request has all the attributes
of the request set to the same values as in the request. It may have
additional attributes, but all attributes in the request are set 
and have the right value.

*Wildcarding* is naturally done by not specifying the attribute in the
request.

## ToDo / goals ##

  * Provide CLI tools to create and query a database.
  * use xml and/or json for queries.

## maybe some time... ##

  * **more logic in requests**: 
    specifiers like "not", "one-of", etc. to attributes might be 
    interesting.
  * **notice if the file has changed**
    save the file mtime and checksum in the database and 
    update / refuse if the index is not in sync with the file.
  * **inter-file support**
    make databases cover several files: The database can then
    also be used to create file lists. 
    Example: "get all messpec data (kappa=...) of H101" returns a list
    of files and datasets that return relevant data.
