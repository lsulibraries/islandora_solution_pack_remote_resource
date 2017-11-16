<?php

class IslandoraRemoteResourceOaiDoraBatchPreprocessor extends IslandoraRemoteResourceListBatchPreprocessor {

  protected function getUrlList() {
    $oaiListUrl = sprintf('%s?verb=ListRecords&metadataPrefix=mods&set=%s', $this->parameters['oai_endpoint'], $this->parameters['oai_set']);
    $recordsList = file_get_contents($oaiListUrl);
    $xml = simplexml_load_string($recordsList);
    $xml->registerXPathNamespace('o', "http://www.openarchives.org/OAI/2.0/");

    $identifiers = $xml->xpath('//o:identifier');
    $urls = array();
    $remote_host = $this->parameters['oai_remote_host'];

    foreach ($identifiers as $id) {
      $i = $this->parameters['oai_pid_index'];
      $pid = explode(':', $id)[$i];
      $urls[] = sprintf("%s/islandora/object/%s", $remote_host, preg_replace('/_/', ':', $pid));
    }
    return $urls;
  }
}
