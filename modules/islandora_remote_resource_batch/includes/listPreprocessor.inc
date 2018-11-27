<?php

class IslandoraRemoteResourceListBatchPreprocessor extends IslandoraRemoteResourceBatchPreprocessor {

  public function preprocess() {
    module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');

    $added = array();
    $urls = $this->getUrlList();
    foreach ($urls as $remote_url) {
      $log_leader = "Processing URL $remote_url - ";
      $search_url = preg_replace("#http[s]?://#", '', $remote_url);
      $pid = islandora_remote_resource_batch_query_solr($search_url);
      $object = islandora_object_load($pid);
      $solr_ghost = $pid !== FALSE && $object == FALSE;

      // We only want to ingest objects that aren't already in our repo.
      if (!$pid || $solr_ghost) {
        $batch_object = new IslandoraRemoteResourceDoraListBatchObject($this->connection, $remote_url, $this->parameters);
        // Add the instances to the Islandora batch queue.
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
        drush_log("$log_leader NEW    - Added new object", 'ok');
      }
      else {
        $remote_mods = $this->getRemoteDatastream('MODS', $remote_url);
        if (md5($remote_mods) != md5($object['MODS']->content)) {
          drush_log("$log_leader UPDATE - metadata for $pid", 'ok');
          $dsid = 'MODS';
          // Before we do anything, check if the datastream exists. If it does, load it; otherwise construct it.
          // The easiest way to do this, as opposed to a string of cases or if/then/elses, is the ternary operator, e.g.
          // $variable = isThisThingTrueOrFalse($thing) ? setToThisIfTrue() : setToThisIfFalse();
          if (!isset($object[$dsid])) {
            $datastream = $object->constructDatastream($dsid, 'M');
            $datastream->label = 'MODS metadata';
            $datastream->mimeType = 'application/xml';
            $datastream->setContentFromString($remote_mods);
            $object->ingestDatastream($datastream);
          }
          else {
            $datastream = $object[$dsid];
            $datastream->setContentFromString($remote_mods);
          }

          if (module_exists('islandora_batch_derivative_trigger')) {
            module_load_include('inc', 'islandora_batch_derivative_trigger', 'includes/batch');
            islandora_batch_derivative_trigger_regenerate_dc_metadata($object, 'mods_to_dc.xsl:MODS');
          }
        }
	else {
	  drush_log(sprintf("%s SKIP   - No update needed for %s", $log_leader, $pid), 'ok');
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
