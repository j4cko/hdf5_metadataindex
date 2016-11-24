#include "postselection.h"
#include <iostream>

void filterIndexByPostselectionRules(Index& idx, Request const & req) {
  idx.erase( std::remove_if( idx.begin(), idx.end(),
    [&req](DatasetSpec const & dsetspec) {
      bool matches = true; //preselection has already matched..
      // now, run over all attributeRequests: every request must match against
      // any attribute:
      for( auto const & attrreq : req ) {
        bool thisReqIsFulfilled = false;
        for( auto const & attr : dsetspec.attributes )
          thisReqIsFulfilled |= attrreq.matches(attr);
        // no need to look further: remove the datasetspec...
        if( not thisReqIsFulfilled ) return true;
        matches &= thisReqIsFulfilled;
      }
      //remove if none of the attributes could match the conditions in the
      //request.
      if( not matches ) {
        std::cout << "attribute " << dsetspec.datasetname << " is removed in postfiltering." << std::endl;
      }
      return not matches;
    }), idx.end());
}
