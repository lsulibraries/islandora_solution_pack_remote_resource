<?php

class IslandoraRemoteResourceBatchListPreprocessor extends IslandoraRemoteResourceBatchPreprocessor {

  public function preprocess() {
    module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');

    $added = array();
    if (isset($this->parameters['list'])) {
      $input_paths = $this->getListBatch();
    }
    // oai
    else {
      
      $oaiListUrl = sprintf('%s?verb=ListRecords&metadataPrefix=mods&set=%s', $this->parameters['oai_endpoint'], $this->parameters['oai_set']);
      $recordsList = file_get_contents($oaiListUrl);
      $xml = simplexml_load_string($recordsList);
      $xml->registerXPathNamespace('o', "http://www.openarchives.org/OAI/2.0/");

      $identifiers = $xml->xpath('//o:identifier');
      $input_paths = array();
      $remote_host = $this->parameters['oai_remote_host'];

      foreach ($identifiers as $id) {
        $pid = explode(':', $id)[3];
        $input_paths[] = sprintf("%s/islandora/object/%s", $remote_host, preg_replace('/_/', ':', $pid));
      }

    }

    foreach ($input_paths as $remote_url) {
      // We only want to ingest objects that aren't already in our repo.
      if (!islandora_remote_resource_batch_query_solr($remote_url)) {
        $batch_object = new IslandoraRemoteResourceBatchListObject($this->connection, $remote_url, $this->parameters);
        // Add the instances to the Islandora batch queue.
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
    }

    return $added;
  }

  protected function getListBatch() {
    return explode("\n", file_get_contents($this->parameters['list']));
  }

}
