<?php

class IslandoraRemoteResourceDoraListBatchObject extends IslandoraBatchObject {
  protected $url;
  protected $preprocessorParameters;

  public function __construct(IslandoraTuque $connection, $remote_url, $parameters) {
    parent::__construct(NULL, $connection->repository);

    $this->label = 'Untitled';
    $this->url = $remote_url;

    $this->preprocessorParameters = $parameters;
  }

  /**
   * Assemble the base datastreams.
   */
  public function batchProcess() {
    module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');

    // OBJ datastream.
    $obj = $this->constructDatastream('OBJ', 'M');
    $obj->label = 'Remote URL';
    $obj->mimetype = 'text/plain';
    $obj->setContentFromString($this->url, FALSE);
    $this->ingestDatastream($obj);

    // MODS, then from it DC, datastreams, if we have files named x.MODS.xml.
    if ($mods = $this->getMods()) {
      $this->ingestDatastream($mods);
      $path_to_mods_to_dc_xsl = drupal_get_path('module', 'islandora_batch') . '/transforms/mods_to_dc.xsl';
      $mods_to_dc_xsl = file_get_contents($path_to_mods_to_dc_xsl);
      $dc_xml = islandora_remote_resource_batch_apply_xslt($mods_to_dc_xsl, $mods->content);
      if ($dc = $this->getDcFromMods($dc_xml)) {
        $this->ingestDatastream($dc);
      }
    }

// If the input data contains DC datastreams, load them.
    $this->getDc();

    $this->label = islandora_solution_pack_remote_resource_get_label_from_dc($this['DC']->content);

// TN datastream.
    if ($tn = $this->getTn()) {
      $this->ingestDatastream($tn);
    }

// Additional datastreams.
    $additional_datastreams = $this->getAdditionalDatastreams();
    if (count($additional_datastreams)) {
      foreach ($additional_datastreams as $additional) {
        $this->ingestDatastream($additional);
      }
    }

    $this->addRelationships();

// Indicate that this object is ready to be ingested.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Add relationships.
   */
  public function addRelationships() {
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $this->preprocessorParameters['parent']);
    $this->models = 'islandora:sp_remote_resource';
  }

  /**
   * Get a list of resources.
   */
  public function getResources() {
    return array();
  }

  private function getDSurl($dsid) {
    return sprintf('%s/datastream/%s/view', $this->url, $dsid);
  }
  
  /**
   * Creates a DC datastream from a file.
   *
   * Note: The DC datastream does not contain the local PID.
   *
   * @return object
   *   The DC datastream, ready for ingestion.
   */
  private function getDc() {
    $this['DC']->setContentFromString(
        file_get_contents($this->getDSurl('DC')), FALSE);
  }

  /**
   * Creates a TN datastream from a file matching the base object.
   *
   * @return object|FALSE
   *   A datastream object for the thumbnail, or FALSE if no datastream
   *   was created.
   */
  private function getTn() {
    if (TRUE) {
      $tn_mime_detector = new MimeDetect();
      $tn = $this->constructDatastream('TN', 'M');
      $tn->mimetype = $tn_mime_detector->getMimetype($tn_raw);
      $tn->label = 'Thumbnail';
      $tn->setContentFromUrl($this->getDSurl('TN'), FALSE);
      return $tn;
    }
    else {
      $default_tn_path = variable_get('islandora_solution_pack_remote_resource_default_thumbnail_path', '');
      if (strlen($default_tn_path) && file_exists($default_tn_path)) {
        $tn_mime_detector = new MimeDetect();
        $tn = $this->constructDatastream('TN', 'M');
        $tn->mimetype = $tn_mime_detector->getMimetype($default_tn_path);
        $tn->label = 'Thumbnail';
        $tn->setContentFromFile($default_tn_path, FALSE);
        return $tn;
      }
    }
    return FALSE;
  }

  /**
   * Creates a MODS datastream.
   *
   * If a file in the same directory as the OBJ file and matching its basename
   * plus '.MODS.xml' exists, populates the MODS datastream with that file;
   * if no such file exists, generates a minimal, templated MODS datastream.
   *
   * @return obj|FALSE
   *   A datastream object for the MODS document, or FALSE if no datastream
   *   was created.
   */
  private function getMods() {
    $mods = $this->constructDatastream('MODS', 'M');
    $mods->mimetype = 'application/xml';
    $mods->label = 'MODS metadata';
  
    $mods->setContentFromString(
        file_get_contents($this->getDSurl('MODS')), FALSE);

    if(!empty($mods->content)) {
      return $mods;
    }
    else {
// In the absence of a poper MODS file, generate a minimal one.
      $mods = $this->constructDatastream('MODS', 'M');
      $mods->content = <<<EOQ
<mods xmlns='http://www.loc.gov/mods/v3'>
  <titleInfo>
    <title>{$file_name}</title>
  </titleInfo>
</mods>
EOQ;
      $mods->mimetype = 'application/xml';
      $mods->label = 'MODS metadata';
      return $mods;
    }
    return FALSE;
  }

  /**
   * Generates a DC datastream from MODS XML.
   *
   * @param string $dc_xml
   *   The serialized DC XML.
   *
   * @return object
   *   The DC datastream, ready for ingestion.
   */
  private function getDcFromMods($dc_xml) {
    $dc = $this->constructDatastream('DC', 'M');
    $dc->mimetype = 'application/xml';
    $dc->label = 'DC Record';
    $dc->setContentFromString($dc_xml);
    return $dc;
  }

  /**
   * Creates any additional datastreams for the object based on filename.
   *
   * For a base OJB file with the name somefile.txt, a datastream with the
   * DSID 'FOO' will be created from a file with the name somfile.FOO.ext.
   * The datastream's mimetype will be derived from .ext. Its label will
   * be 'FOO datastream'.
   *
   * @return array
   *   A list of additional datastreams to ingest with the current object.
   */
  private function getAdditionalDatastreams() {
    $datastreams = array();

    $pathinfo = pathinfo($this->txtObjFilePath);
    $dir_name = $pathinfo['dirname'];
    $file_name = $pathinfo['filename'];
    $base_path = $dir_name . DIRECTORY_SEPARATOR . $file_name;

    $ds_filepaths = glob($base_path . '*');
    foreach ($ds_filepaths as $ds_filepath) {
// We don't want the base .txt file.
      if ($ds_filepath != $base_path . '.txt') {
// Or the DC, MODS, TN, files since those datastreams are
// created elsewhere.
        if (!preg_match('/\.(DC|TN|MODS)/', $ds_filepath)) {
          $ds_filepath_filename = pathinfo($ds_filepath, PATHINFO_FILENAME);
// The DSID is not an extension, but it will be the part after
// the final . in $ds_filepath_filename.
          $dsid = pathinfo($ds_filepath_filename, PATHINFO_EXTENSION);
          if (strlen($dsid)) {
            $ds = $this->constructDatastream($dsid, 'M');
            $mime_detector = new MimeDetect();
            $ds->mimetype = $mime_detector->getMimetype($ds_filepath);
            $ds->label = $dsid . ' datastream';
            $ds->setContentFromFile($ds_filepath, FALSE);
            $datastreams[] = $ds;
          }
        }
      }
    }
    return $datastreams;
  }

}