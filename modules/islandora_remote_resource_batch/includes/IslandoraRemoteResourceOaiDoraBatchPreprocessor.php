<?php

class IslandoraRemoteResourceOaiDoraBatchPreprocessor extends IslandoraRemoteResourceListBatchPreprocessor {

  protected function getUrlList() {
    $oaiListUrl = sprintf('%s?verb=ListIdentifiers&metadataPrefix=mods&set=%s', $this->parameters['oai_endpoint'], $this->parameters['oai_set']);
    $recordsList = file_get_contents($oaiListUrl);
    $xml = simplexml_load_string($recordsList);
    $xml->registerXPathNamespace('o', "http://www.openarchives.org/OAI/2.0/");

    $identifiers = $xml->xpath('//o:identifier');
    $urls = array();
    $remote_host = $this->parameters['oai_remote_host'];
    drush_log('Building URL list...', 'ok');
    do {
      foreach ($identifiers as $id) {
        $i = $this->parameters['oai_pid_index'];
        $pid = explode(':', $id)[$i];
        $urls[] = sprintf("%s/islandora/object/%s", $remote_host, preg_replace('/_/', ':', $pid));
      }
      $countSoFar = count($urls);
      
      $resumptionToken = $xml->ListIdentifiers->resumptionToken;
      if ($resumptionToken) {
        $completeListSize = $resumptionToken['completeListSize'];
        drush_log(sprintf("%s / %s", $countSoFar, $completeListSize), 'ok');
        $resumeUrl = sprintf('%s?verb=ListIdentifiers&resumptionToken=%s', $this->parameters['oai_endpoint'], $resumptionToken);
        $recordsList = file_get_contents($resumeUrl);
        $xml = simplexml_load_string($recordsList);
        $xml->registerXPathNamespace('o', "http://www.openarchives.org/OAI/2.0/");
        $identifiers = $xml->xpath('//o:identifier');
      }
    } while ($resumptionToken);
    return $urls;
  }
}
