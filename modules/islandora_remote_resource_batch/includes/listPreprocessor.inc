<?php

class IslandoraRemoteResourceListBatchPreprocessor extends IslandoraRemoteResourceBatchPreprocessor {

  public function preprocess() {
    module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');

    $added = array();
    $urls = $this->getUrlList();
    foreach ($urls as $remote_url) {
      $pid = islandora_remote_resource_batch_query_solr($remote_url);
      // We only want to ingest objects that aren't already in our repo.
      if (!$pid) {
        $batch_object = new IslandoraRemoteResourceDoraListBatchObject($this->connection, $remote_url, $this->parameters);
        // Add the instances to the Islandora batch queue.
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
      else {
        $object = islandora_object_load($pid);
        $remote_mods = $this->getRemoteDatastream('MODS', $remote_url);
        if (md5($remote_mods) != md5($object['MODS'])) {
          $object['MODS']->setContentFromString($remote_mods);
        }
      }
    }
    return $added;
  }

  protected function getUrlList() {
    return explode("\n", file_get_contents($this->parameters['list']));
  }

  protected function getRemoteDatastream($dsid, $remote_url) {
      $url = $this->getDSurl($dsid, $remote_url);
      try {
          $raw = file_get_contents($url);
      } catch (Exception $e) {
          drush_log($e->getMessage(), 'warning');
      }

      // Try to load the file; skip if not clean XML.
      libxml_use_internal_errors(TRUE);
      $xml = new DOMDocument();
      $xml->loadXML($raw, LIBXML_PEDANTIC);
      $errors = libxml_get_errors();

      if (empty($errors)) {
          return $raw;
      } else {
          $log_messages = [];
          foreach ($errors as $error) {
              drush_log(
                      sprintf("Failed getting datastream %s from %s; message was %s", $dsid, $url, $error->message), 'warning');
              $log_messages[] = $error->message;
          }
          watchdog(
                  "Remote Resource Collection", "Failed getting datastream %dsid. Message(s): %mess", array('%dsid' => $dsid, '%mess' => implode(" ---|||--- ", $log_messages)), WATCHDOG_WARNING, $url
          );
          return FALSE;
      }
      libxml_clear_errors();
  }

  private function getDSurl($dsid, $remote_url) {
      return sprintf('%s/datastream/%s/view', $remote_url, $dsid);
  }

}
