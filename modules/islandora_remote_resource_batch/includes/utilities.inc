<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Queries Islandora's Solr index to get an object's PID.
 *
 * @param string $remote_url
 *   The remote URL to query Solr for.
 *
 * @return string|bool
 *   The corresponding PID, or FALSE if none is found.
 */
function islandora_remote_resource_batch_query_solr($remote_url) {
  // See https://git.lib.sfu.ca/mjordan/islandora_solution_pack_remote_resource/issues/7.
  $query = 'OBJ_t:"' . trim($remote_url) . '"';
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $query;
  $query_processor->solrParams['fl'] = 'PID';
  $query_processor->executeQuery(FALSE);
  if ($query_processor->islandoraSolrResult['response']['numFound'] === 1) {
    $pid = $query_processor->islandoraSolrResult['response']['objects'][0]['solr_doc']['PID'];
    return $pid;
  }
  else {
    return FALSE;
  }
}

/**
 * Applies an XSLT transform to an XML string.
 *
 * @param string $xslt
 *   An XSLT stylesheet.
 * @param string $input_xml
 *   An XML string.
 * @param array $params
 *   An associative array of parameters to the stylesheet.
 *
 * @return string
 *   The transformed XML.
 */
function islandora_remote_resource_batch_apply_xslt($xslt, $input_xml, $params = array()) {
  try {
    $xsl_doc = new DOMDocument();
    $xsl_doc->loadXML($xslt);
    $xml_doc = new DOMDocument();
    $xml_doc->loadXML(mb_convert_encoding($input_xml, 'HTML-ENTITIES', 'UTF-8'));
    $xslt_proc = new XSLTProcessor();
    $xslt_proc->importStylesheet($xsl_doc);
    if (count($params)) {
      foreach ($params as $param_name => $param_value) {
        $xslt_proc->setParameter(NULL, $param_name, check_plain($param_value));
      }
    }
    $output_xml = $xslt_proc->transformToXML($xml_doc);
    return $output_xml;
  }
  catch (exception $e) {
    $success = array(
      'success' => FALSE,
      'messages' => array(
        array(
          'message' => t('Failed to apply XSL transform.'),
          'type' => 'watchdog',
          'severity' => WATCHDOG_ERROR,
        ),
      ),
    );
  }
}

/**
 * Get object label from dc:title.
 *
 * @param string $dc
 *   The serialized DC datastream XML.
 *
 * @return string
 *   The label.
 */
function islandora_solution_pack_remote_resource_get_label_from_dc($dc) {
  $dom = new DOMDocument();
  $dom->loadXML($dc);
  $xpath = new DOMXPath($dom);
  $xpath->registerNamespace('dc', 'http://purl.org/dc/elements/1.1/');
  $titles = $xpath->query("//dc:title");
  if ($titles->length > 0) {
    $label = $titles->item(0)->nodeValue;
  }
  else {
    $label = '[No object label available]';
  }
  return $label;
}

/**
 * Attempts to retrieve the remote web page and parse it title.
 *
 * @param string $remote_url
 *   The URL of the remote resource.
 *
 * @return string|bool
 *   The title parsed out from the HTML at the remote URL, or FALSE
 *   if the title could not be parsed.
 */
function islandora_remote_resource_batch_fetch_remote_title($remote_url) {
  $result = drupal_http_request($remote_url);
  if (isset($result->error)) {
    drush_log(dt("Failed to retrieve content from remote resource !url, so cannot get title: !e",
      array('!url' => $remote_url, '!e' => $result->error)), 'warning');
    return FALSE;
  }
  else {
    $code = (string) $result->code;
    if (preg_match('/^2|3/', $code)) {
      libxml_use_internal_errors(TRUE);
      try {
        $dom = new DOMDocument();
        $dom->loadHTML($result->data);
        $titles = $dom->getElementsByTagName('title');
        if ($titles->length > 0) {
          return $titles->item(0)->nodeValue;
        }
        else {
          drush_log(dt("Failed to parse title from remote resource !url: title element not found",
            array('!url' => $remote_url)),
              'warning'
            );
          return FALSE;
        }
      }
      catch (exception $e) {
        drush_log(dt("Failed to parse title from remote resource !url: !em",
          array('!url' => $remote_url, '!em' => $e->getMessage())), 'warning');
      }
    }
    else {
      drush_log(dt("Failed to parse title from remote resource !url: HTTP response code !code",
        array('!url' => $remote_url, '!code' => $code)), 'warning');
      return FALSE;
    }
  }
}
