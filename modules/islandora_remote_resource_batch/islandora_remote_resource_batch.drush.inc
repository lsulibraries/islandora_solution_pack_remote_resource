<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_remote_resource_batch_drush_command() {
  $items = array();

  $items['islandora_remote_resource_create_oai_collections'] = array(
    'aliases' => array('irrccoai'),
    'description' => 'Create local collections based on the set of collections returned from an oai endpoint.',
    'drupal dependencies' => array('islandora', 'islandora_basic_collection', 'islandora_solution_pack_remote_resource', 'islandora_utils'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'options' => array(
      'oai_endpoint' => array(
        'description' => 'URL of the OAI endpoint.',
        'value' => 'required',
      ),
      'oai_remote_host' => array(
        'description' => 'Base URL of the OAI endpoint.',
        'value' => 'required',
      ),
      'oai_endpoint_protocol' => array(
        'description' => "whether to use http (default) or https.",
        'value' => 'optional',
      ),
    )
  );
  
  $items['islandora_remote_resource_batch_preprocess'] = array(
    'aliases' => array('irrbp'),
    'description' => 'Preprocess Islandora Remote Resource objects into batch queue entries.',
    'drupal dependencies' => array('islandora_batch', 'islandora_solution_pack_remote_resource'),
    'options' => array(
      'namespace' => array(
        'description' => 'The namespace for objects created by this command.',
        'required' => TRUE,
      ),
      'list_file' => array(
        'description' => 'File containing a list of URLs, one per line.',
        'required' => FALSE,
      ),
      'parent' => array(
        'description' => 'The PID of the collection to which the generated items should be added.',
        'required' => TRUE,
      ),
      'parent_relationship_uri' => array(
        'description' => 'The namespace URI of the relationship to the parent. Defaults to "info:fedora/fedora-system:def/relations-external#".',
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => 'The predicate of the relationship to the parent. Defaults to "isMemberOf".',
        'value' => 'optional',
      ),
      'oai_endpoint' => array(
        'description' => 'URL of the OAI endpoint.',
        'value' => 'optional',
      ),
      'oai_remote_host' => array(
        'description' => 'Base URL of the OAI endpoint.',
        'value' => 'optional',
      ),
      'oai_set' => array(
        'description' => 'Set id.',
        'value' => 'optional',
      ),
      'oai_pid_index' => array(
        'description' => 'When exploding oai identifier value by ":", which array index holds the pid ?',
        'value' => 'optional',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  if (DRUSH_VERSION >= 7) {
    $items['islandora_remote_resource_batch_preprocess']['options']['scan_target'] = array(
      'description' => 'Absolute path to the target directory to scan.',
    );
  }
  else {
    $items['islandora_remote_resource_batch_preprocess']['options']['target'] = array(
      'description' => 'Absolute path to the target directory to scan.',
    );
  }

  // Does not use the Batch API. Included in the Remote Resource Batch module
  // anyway. URL value is indexed in the Solr field 'OBJ_t'.
  $items['islandora_remote_resource_batch_sync'] = array(
    'aliases' => array('irrbs'),
    'description' => 'Updates datastreams (other than OBJ) in Islandora Remote Resource objects.',
    'drupal dependencies' => array('islandora_batch', 'islandora_solution_pack_remote_resource'),
    'options' => array(
      'target' => array(
        'description' => 'The target directory to scan.',
        'required' => TRUE,
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $items;
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_remote_resource_batch_preprocess() {
  // Due to how Drush bootstrapping works, the connection may be created
  // without credentials (when your site's front page is
  // 'islandora/object/some:object', for example). Resetting to ensure a new
  // connection gets created should fix it.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'namespace' => drush_get_option('namespace'),
    'target' => drush_get_option('scan_target'),
    'list' => drush_get_option('list_file'),
    'parent' => drush_get_option('parent'),
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', 'info:fedora/fedora-system:def/relations-external#'),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isMemberOf'),
    'oai_endpoint' => drush_get_option('oai_endpoint'),
    'oai_remote_host' => drush_get_option('oai_remote_host'),
    'oai_set' => drush_get_option('oai_set'),
    'oai_pid_index' => drush_get_option('oai_pid_index', 2),
  );

  // Run the preprocessor.
  $class = get_preprocessor($parameters);
  $preprocessor = new $class($connection, $parameters);
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  drush_log(t("SetId: @s", array('@s' => $preprocessor->getSetId())), "ok");
}

function get_preprocessor($parameters) {
  if (isset($parameters['target'])) {
    return 'IslandoraRemoteResourceBatchPreprocessor';
  }
  
  $oaiBased = isset($parameters['oai_endpoint']) && isset($parameters['oai_remote_host']) && isset($parameters['oai_set']);
  $listBased = isset($parameters['list']);
  if ($oaiBased xor $listBased) {
    if ($oaiBased) {
      return 'IslandoraRemoteResourceOaiDoraBatchPreprocessor';
    }
    else {
      return 'IslandoraRemoteResourceListBatchPreprocessor';
    }
  }
  else {
    throw new Exception ("Only one of ('target' || 'list' || 'oai*') can be set.");
  }
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_remote_resource_batch_sync() {
  module_load_include('inc', 'islandora_remote_resource_batch', 'includes/sync');
  $dir_to_scan = drush_get_option('target');
  islandora_remote_resource_batch_sync($dir_to_scan);
}

/**
 * Parent collection validation.
 */
function drush_islandora_remote_resource_batch_preprocess_validate() {
  $parent = drush_get_option('parent');
  $object = islandora_object_load($parent);
  if (!$object) {
    return drush_set_error('This is not a valid parent', dt('The specified parent (@parent) was unable to be loaded.', array('@parent' => $parent)));
  }
}

function drush_islandora_remote_resource_batch_islandora_remote_resource_create_oai_collections() {
  $ssl = drush_get_option('oai_endpoint_protocol', 'http');
  if(!in_array(strtolower($ssl), array('http', 'https'))) {
    drush_die("Illegal value $ssl given for oai_endpoint_protocol; must be one of [http|https]", 'error');
  }
  $oai = drush_get_option('oai_endpoint');
  $url = sprintf("http://%s?verb=ListSets", $oai);

  $xml = simplexml_load_string(file_get_contents($url));
  $ns = $xml->getNamespaces(TRUE);
  foreach($ns as $alias => $uri) {
    $xml->registerXPathNamespace($alias, $uri);
  }
  $xml->registerXPathNamespace('oai', "http://www.openarchives.org/OAI/2.0/");
  $sets = $xml->xpath('//oai:set');

  foreach($sets as $set) {
    $description_wrapper = $set->setDescription->children('oai_dc', TRUE);
    $description = $description_wrapper->children('dc', TRUE);
  
    module_load_include('inc', 'islandora_utils', 'includes/util');
    $namespace = str_replace('_', ':', $set->setSpec);
    if($namespace == 'islandora:root') {
      continue;
    }
    islandora_utils_ingest_collection($description->title, $description->description, $namespace, 'islandora:root');
  }
}