#include "postselection.h"
#include <iostream>

void filterIndexByAttributeRequests(Index& idx, std::vector<AttributeRequest> const & req) {
  idx.erase( std::remove_if( idx.begin(), idx.end(),
    [&req](DatasetSpec const & dsetspec) {
      bool matches = true; //preselection has already matched..
      // run over all attributeRequests: every request must match against
      // any attribute:
      for( auto const & attrreq : req) {
        bool thisReqIsFulfilled = false;
        for( auto const & attr : dsetspec.attributes )
          thisReqIsFulfilled |= attrreq.matches(attr);
        // no need to look further: remove the datasetspec...
        if( not thisReqIsFulfilled ) return true;
        matches &= thisReqIsFulfilled;
      }
      //remove if none of the attributes could match the conditions in the
      //request.
      return (not matches);
    }), idx.end());
}
void filterIndexByFileRequests(Index& idx, std::vector<FileRequest> const & req) {
  idx.erase( std::remove_if( idx.begin(), idx.end(),
    [&req](DatasetSpec const & dsetspec) {
      bool matches = true;
      for( auto const & filereq : req ) {
        matches &= filereq->matches(dsetspec.file);
      }
      return (not matches);
    }), idx.end());
}
void filterIndexByHdf5DatasetRequests(Index& idx, std::vector<Hdf5DatasetRequest> const & req) {
  idx.erase( std::remove_if( idx.begin(), idx.end(),
    [&req](DatasetSpec const & dsetspec) {
      bool matches = true;
      for( auto const & dsetreq : req ) {
        matches &= dsetreq->matches(dsetspec.datasetname, dsetspec.location);
      }
      return (not matches);
    }), idx.end());
}
void filterIndexByPostselectionRules(Index& idx, Request const & req) {
  filterIndexByHdf5DatasetRequests(idx, req.dsetrequests);
  filterIndexByAttributeRequests(idx, req.attrrequests);
  filterIndexByFileRequests(idx, req.filerequests);
}
