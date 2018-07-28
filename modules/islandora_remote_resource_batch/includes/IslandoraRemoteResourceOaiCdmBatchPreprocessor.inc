<?php

/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

class IslandoraRemoteResourceOaiCdmBatchPreprocessor extends IslandoraRemoteResourceOaiCollectionBatchPreprocessor {
  
  protected function getMetadataPrefix() {
    return 'oai_qdc';
  }

  protected function getUrlForId($identifier) {
    // http://cdm16340.contentdm.oclc.org/cdm/ref/collection/p16340coll12/id/1
    $remote_host = $this->parameters['oai_remote_host'];
    $url = sprintf("%s/cdm/ref/collection/%s/id/%s", $remote_host, $this->parameters['oai_set'], $identifier);
    drush_log($url, 'info');
    return $url;
  }
  
  protected function getIdfromIdentifier($identifier) {
    $pos = strpos($identifier, '/');
    $id = substr($identifier, $pos+1);
    return $id;
  }

  protected function getObjectClass() {
    return 'IslandoraRemoteResourceCdmListBatchObject';
  }

  protected function getIdentifiers() {
    $this->parameters['dc_records'] = [];
    $recs = $this->remote_collection->records();
    foreach($recs as $rec) {
      $oai = $rec->children("http://www.openarchives.org/OAI/2.0/");
      $id = (string) $rec->header->identifier;
      if ($rec->header->metadata) {
	$dc = $oai->metadata->children('oai_qdc', TRUE)->qualifieddc->asXML();
	$this->parameters['dc_records'][$id] = $dc;
      }
      else {
	// No metadata, as in p15196coll2/37
	continue;
      }
    }
    $identifiers = array_keys($this->parameters['dc_records']);
    return $identifiers;
  }
}