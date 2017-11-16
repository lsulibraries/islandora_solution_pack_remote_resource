<?php

class IslandoraRemoteResourceListBatchPreprocessor extends IslandoraRemoteResourceBatchPreprocessor {

  public function preprocess() {
    module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');

    $added = array();
    $urls = $this->getUrlList();

    foreach ($urls as $remote_url) {
      // We only want to ingest objects that aren't already in our repo.
      if (!islandora_remote_resource_batch_query_solr($remote_url)) {
        $batch_object = new IslandoraRemoteResourceDoraListBatchObject($this->connection, $remote_url, $this->parameters);
        // Add the instances to the Islandora batch queue.
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
    }
    return $added;
  }

  protected function getUrlList() {
    return explode("\n", file_get_contents($this->parameters['list']));
  }
}
