<?php

class IslandoraRemoteResourceOaiCollectionBatchPreprocessor extends IslandoraRemoteResourceListBatchPreprocessor {

  public $parometers = array();
  public $remote_collection;

  public function __construct($connection, $parameters, $remote_collection) {
    parent::__construct($connection, $parameters);
    $this->target_collection = $parameters['target_collection'];
    $this->parent_collection = $parameters['parent_collection'];
    $this->remote_collection = $remote_collection;
  }
  
  protected function getUrlList() {
    $identifiers = $this->remote_collection->identifiers();
    $urls = array();
    foreach($identifiers as $record) {
      $pid = str_replace('_', ':', explode(':', $record->identifier)[3]);
      (string) $base = $this->remote_collection->hostBaseUrl();
      $url = sprintf("%s/islandora/object/%s", $base, $pid);
      $urls[] = $url;
    }
    return $urls;
  }
}
