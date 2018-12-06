<?php

require_once drupal_realpath(dirname(dirname(__FILE__))) . '/vendor/autoload.php';

class OaiObject {

}

class OaiCollection {

  public $setSpec, $endpoint, $protocol, $host, $oai_path, $description, $title, $since;

  public function __construct($set, $protocol, $host, $oai_path, $since = NULL) {

    if (NULL !== $since) {
      $since = new \DateTime($since);
    }

    $this->since = $since;
    $this->setSpec  = $set;
    $this->protocol = trim($protocol, '/');
    $this->host     = trim($host, '/');
    $this->oai_path = trim($oai_path, '/');
    $this->endpoint = \Phpoaipmh\Endpoint::build($this->endpointUrl());
    $this->init_collection_meta();
  }

  public function endpointUrl() {
    return sprintf("%s://%s/%s", $this->protocol, $this->host, $this->oai_path);
  }

  public function hostBaseUrl() {
    return sprintf("%s://%s", $this->protocol, $this->host);
  }

  public function identifiers() {
    $recs = $this->endpoint->listIdentifiers('mods', $this->since, NULL, $this->setSpec);
    return $recs;
  }

  public function init_collection_meta() {
    $sets = $this->endpoint->listSets();
    foreach ($sets as $set) {
      drush_log($set, 'info');
      if($set->setSpec == $this->setSpec)  {
        break;
      }
    }
    $this->title = $set->setName;
    $description_wrapper = $set->setDescription->children('oai_dc', TRUE);
    $description = $description_wrapper->children('dc', TRUE);
    $this->description = str_replace('&', '&amp;', $description->description);
    //    $this->title = str_replace('&', '&amp;', $description->title);
  }
}

class OaiEndpoint {

  public $protocol, $host, $path;

  public function __construct($p, $h, $pth) {
    $this->protocol = trim($p, '/');
    $this->host = trim($h, '/');
    $this->path = trim($pth, '/');
  }

  public function queryBaseUrl() {
    return sprintf("%s://%s/%s", $this->protocol, $this->host, $this->path);
  }

  public function hostBaseUrl() {
    return sprintf("%s://%s", $this->protocol, $this->host);
  }

}
