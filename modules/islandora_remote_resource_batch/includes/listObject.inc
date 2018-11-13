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

    private function getDSurl($dsid) {
        return sprintf('%s/datastream/%s/view', $this->url, $dsid);
    }

    protected function getRemoteDatastream($dsid) {
        $url = $this->getDSurl($dsid);
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

    /**
     * Assemble the base datastreams.
     */
    public function batchProcess() {
        drush_log(sprintf("Processing %s", $this->url), 'ok');
        module_load_include('inc', 'islandora_remote_resource_batch', 'includes/utilities');
        // OBJ datastream.
        $obj = $this->constructDatastream('OBJ', 'M');
        $obj->label = 'Remote URL';
        $obj->mimetype = 'text/plain';
        $obj->setContentFromString($this->url, FALSE);
        $this->ingestDatastream($obj);

        // MODS, then from it DC, datastreams, if we have files named x.MODS.xml.
        $mods = $this->getMods();
        if ($mods) {
            $this->ingestDatastream($mods);
            $path_to_mods_to_dc_xsl = drupal_get_path('module', 'islandora_batch') . '/transforms/mods_to_dc.xsl';
            $mods_to_dc_xsl = file_get_contents($path_to_mods_to_dc_xsl);
            $dc_xml = islandora_remote_resource_batch_apply_xslt($mods_to_dc_xsl, $mods->content);
            $dc = $this->getDcFromMods($dc_xml);
            if ($dc) {
                $this->ingestDatastream($dc);
            }
        } else {
            return ISLANDORA_BATCH_STATE__ERROR;
        }

        $this->label = islandora_solution_pack_remote_resource_get_label_from_dc($this['DC']->content);

        // TN datastream.
        if ($tn = $this->getTn()) {
            $this->ingestDatastream($tn);
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

    /**
     * Creates a TN datastream from a file matching the base object.
     *
     * @return object|FALSE
     *   A datastream object for the thumbnail, or FALSE if no datastream
     *   was created.
     */
    private function getTn() {
        $fetched = FALSE;
        $url = $this->getDSurl('TN');
        try {
            $fetched = file_get_contents($url);
        } catch (Exception $e) {
            drush_log(sprintf("Trouble fetching TN for %s; message was: %s", $url, $e->getMessage()), 'warn');
        }
        if ($fetched) {
            $tn_mime_detector = new MimeDetect();
            $tn = $this->constructDatastream('TN', 'M');
            $tn->mimetype = $tn_mime_detector->getMimetype($fetched);
            $tn->label = 'Thumbnail';
            $tn->setContentFromString($fetched, FALSE);
            return $tn;
        } else {
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
        $raw = $this->getRemoteDatastream('MODS');

        if ($raw) {
            $mods->setContentFromString($raw, FALSE);
            if (!empty($mods->content)) {
                return $mods;
            } else {
                // In the absence of a poper MODS file, generate a minimal one.
                $mods = $this->constructDatastream('MODS', 'M');
                $mods->content = <<<EOQ
<mods xmlns='http://www.loc.gov/mods/v3'>
  <titleInfo>
    <title>{$this->url}</title>
  </titleInfo>
</mods>
EOQ;
                $mods->mimetype = 'application/xml';
                $mods->label = 'MODS metadata';
                return $mods;
            }
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
}
