<?php

class IslandoraRemoteResourceCdmListBatchObject extends IslandoraRemoteResourceDoraListBatchObject {

  protected function ingestMetadata() {
    $identifier = $this->preprocessorParameters['identifiers'][$this->url];
    $dc = $this->preprocessorParameters['dc_records'][$identifier];
    $dc_datastream = $this->buildMetadataDatastream($dc, 'DC');
    $this->ingestDatastream($dc_datastream);
    /*
    $dc2mods_path = drupal_get_path('module', 'islandora_remote_resource_batch') . '/transforms/dc2mods.xsl';
    $dc2mods_xsl = file_get_contents($dc2mods_path);
    $mods_xml = islandora_remote_resource_batch_apply_xslt($dc2mods_xsl, $dc);
    $mods = $this->buildMetadataDatastream($mods_xml, 'MODS');
    $this->ingestDatastream($mods);
    */
  }

  protected function getDSurl($dsid) {
    if ('TN' !== $dsid) {
      return;
    }
    list($alias, $id) = $this->getAliasAndIdForIdentifier();
    $host = $this->preprocessorParameters['oai_host'];
//    var_dump($this->preprocessorParameters); die();
    return "http://$host/utils/getthumbnail/collection/$alias/id/$id";
  }

  protected function getAliasAndIdForIdentifier() {
    $parts  = explode('/', $this->url);
    $id = array_pop($parts);
    array_pop($parts);
    $alias = array_pop($parts);
    return [$alias, $id];
  }
}