<?php

class IslandoraRemoteResourceOaiCollectionBatchPreprocessor extends IslandoraRemoteResourceListBatchPreprocessor {

  public $parameters = array();
  public $remote_collection;

  public function __construct($connection, $parameters, $remote_collection) {
    parent::__construct($connection, $parameters);
    $this->target_collection = $parameters['target_collection'];
    $this->parent_collection = $parameters['parent_collection'];
    $this->remote_collection = $remote_collection;
  }
  
  protected function getUrlList() {
    $identifiers = $this->getIdentifiers();
    $this->parameters['identifiers'] = [];
    $urls = array();
    foreach($identifiers as $identifier) {
      $id = $this->getIdfromIdentifier($identifier);
      $url = $this->getUrlForId($id);
      printf("adding url $url\n");
      $urls[] = $url;
      $this->parameters['identifiers'][$url] = $identifier;
    }
    return $urls;
  }
  
  protected function getUrlForId($id) {
    (string) $base = $this->remote_collection->hostBaseUrl();
    $url = sprintf("%s/islandora/object/%s", $base, $id);
    return $url;
  }

  protected function getMetadataPrefix() {
    return 'mods';
  }
  
  protected function getIdfromIdentifier($identifier) {
    $id = str_replace('_', ':', explode(':', $identifier->identifier)[$i]);
    return $id;
  }

  protected function getIdentifiers() {
    return $this->remote_collection->identifiers();
  }
}
