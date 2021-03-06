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

  $items['islandora_remote_resource_ingest_remote_oai_collection'] = array(
    'aliases' => array('irrcc'),
    'description' => 'Create local collection set to receive Remote Resource content.',
    'drupal dependencies' => array(
      'islandora',
      'islandora_basic_collection',
      'islandora_solution_pack_remote_resource',
      'islandora_utils',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'options' => array(
      'oai_endpoint_protocol' => array(
        'description' => "whether to use http (default) or https.",
        'value' => 'optional',
      ),
      'oai_host' => array(
        'description' => 'hostname of the oai endpoint, to be used in constructing request urls.',
        'required' => TRUE,
      ),
      'oai_endpoint' => array(
        'description' => 'path to the OAI endpoint; for example /oai2',
        'required' => TRUE,
      ),
      'oai_set' => array(
        'description' => 'OAI set identifier',
        'required' => TRUE,
      ),
      'oai_remote_platform' => [
        'description' => 'Type of site are we harvesting from- either \'cdm\' or \'Islandora\'',
        'required' => TRUE,
      ],
      'parent' => array(
        'description' => 'PID of local collection where the items should be ingested.',
        'required' => TRUE,
      ),
      'parent_parent_collection' => array(
        'description' => "Collection to which this one should belong [defaults to islandora:root].",
        'value' => "optional",
      ),
      'namespace' => array(
        'description' => 'The namespace for objects ingested into the collection.',
        'required' => TRUE,
      ),
      'since' => array(
        'description' => 'only fetch items changes since the date provided (YYYY-MM-DD).',
        'required' => FALSE,
      ),
    ),
  );
  $items['islandora_remote_resource_batch_islandora_remote_resource_create_collection'] = array(
    'aliases' => array('irrcc'),
    'description' => 'Create local collection set to receive Remote Resource content.',
    'drupal dependencies' => array('islandora', 'islandora_basic_collection', 'islandora_solution_pack_remote_resource', 'islandora_utils'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'options' => array(
      'pid' => array(
        'description' => 'Full PID for the collection',
        'required' => TRUE,
      ),
      'parent' => array(
        'description' => "Collection to which this one should belong; defaults to islandora:root.",
        'required' => TRUE,
      ),
      'title' => array(
        'description' => "Title of the collection.",
        'value' => 'optional',
      ),
      'description' => array(
        'description' => "Description of the collection.",
        'value' => 'optional',
      ),
    ),
  );
  $items['islandora_remote_resource_create_oai_collections'] = array(
    'aliases' => array('irrccoai'),
    'description' => 'Create local collections based on the set of collections returned from an oai endpoint.',
    'drupal dependencies' => array(
      'islandora',
      'islandora_basic_collection',
      'islandora_solution_pack_remote_resource',
      'islandora_utils',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'options' => array(
      'oai_endpoint' => array(
        'description' => 'URL of the OAI endpoint.',
        'required' => TRUE,
      ),
      'oai_remote_host' => array(
        'description' => 'Base URL of the OAI endpoint.',
        'required' => TRUE,
      ),
      'oai_endpoint_protocol' => array(
        'description' => "whether to use http (default) or https.",
        'value' => 'optional',
      ),
    ),
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
  prinf("hello\n------------\n");
  $preprocessor = new $class($connection, $parameters);
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  drush_log(t("SetId: @s", array('@s' => $preprocessor->getSetId())), "ok");
}

function get_preprocessor($parameters) {
  if (isset($parameters['target'])) {
    return 'IslandoraRemoteResourceBatchPreprocessor';
  }
  $oaiBased = isset($parameters['oai_endpoint']) && isset($parameters['oai_set']);
  $listBased = isset($parameters['list']);

  if ($oaiBased xor $listBased) {
    if ($oaiBased) {
      if ($parameters['oai_remote_platform'] == 'islandora') {
        return 'IslandoraRemoteResourceOaiCollectionBatchPreprocessor';
      }
      elseif ($parameters['oai_remote_platform'] == 'cdm') {
        return 'IslandoraRemoteResourceOaiCdmBatchPreprocessor';
      }
      else {
        throw new Exception (sprintf("Expected one of ['cdm'|'islandora']; got %s ", $parameters['oai_remote_platform']));
      }
    }
    else {
      return 'IslandoraRemoteResourceListBatchPreprocessor';
    }
  }
  else {
    throw new Exception("Only one of ('target' || 'list' || 'oai*') can be set.");
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

function drush_islandora_remote_resource_batch_islandora_remote_resource_create_collection() {
  $title = drush_get_option('title', 'no title given');
  $descr = drush_get_option('description', 'no description given');
  $pid = drush_get_option('pid');
  $rent = drush_get_option('parent', 'islandora:root');
  $cmodels = islandora_remote_resource_batch_get_cmodel_for_namespace($pid);
  module_load_include('inc', 'islandora_utils', 'includes/util');
  islandora_utils_ingest_collection($title, $descr, $pid, $rent, $cmodels);
}

function drush_islandora_remote_resource_batch_islandora_remote_resource_ingest_remote_oai_collection() {
  $remote_collection = new OaiCollection(
      drush_get_option('oai_set'), drush_get_option('oai_endpoint_protocol', 'https'), drush_get_option('oai_host'), drush_get_option('oai_endpoint'), drush_get_option('since', NULL)
  );
  $parameters = array(
    'parent' => drush_get_option('parent'),
    // TODO don't hard-code islandora:root
    'parent_parent_collection' => drush_get_option('parent_parent_collection', 'islandora:root'),
    'namespace' => drush_get_option('namespace'),
    'oai_remote_platform' => strtolower(drush_get_option('oai_remote_platform')),
    'oai_host' => drush_get_option('oai_host'),
    'oai_set' => drush_get_option('oai_set'),
    'oai_endpoint' => drush_get_option('oai_endpoint'),
  );
  $existing_collection = islandora_object_load($parameters['parent']);
  if (!$existing_collection) {
    module_load_include('inc', 'islandora_utils', 'includes/util');
    islandora_utils_ingest_collection(
        $remote_collection->title, $remote_collection->description, $parameters['parent'], $parameters['parent_parent_collection'], islandora_remote_resource_batch_get_cmodel_for_namespace($parameters['namespace'])
    );
  }
  // Run the preprocessor.
  $class = get_preprocessor($parameters);
  drupal_static_reset('islandora_get_tuque_connection');
  $preprocessor = new $class(islandora_get_tuque_connection(), $parameters, $remote_collection);
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  drush_log(t("SetId: @s", array('@s' => $preprocessor->getSetId())), "ok");
}

function islandora_remote_resource_batch_get_cmodel_for_namespace($namespace) {
  return array(
    array(
      'dsid' => '',
      'name' => 'Islandora Remote Resource Content Model',
      'namespace' => $namespace,
      'pid' => 'islandora:sp_remote_resource',
    ),
  );
}

function drush_islandora_remote_resource_batch_islandora_remote_resource_create_oai_collections() {
  $proto = drush_get_option('oai_endpoint_protocol', 'http');
  check_protocol($proto);
  $oai = drush_get_option('oai_endpoint');
  $url = sprintf("%s://%s?verb=ListSets", $proto, $oai);
  $xml = simplexml_load_string(file_get_contents($url));
  $ns = $xml->getNamespaces(TRUE);

  foreach ($ns as $alias => $uri) {
    $xml->registerXPathNamespace($alias, $uri);
  }
  $xml->registerXPathNamespace('oai', "http://www.openarchives.org/OAI/2.0/");
  $sets = $xml->xpath('//oai:set');

  $raw_collections = array();
  foreach ($sets as $set) {
    $description_wrapper = $set->setDescription->children('oai_dc', TRUE);
    $description = $description_wrapper->children('dc', TRUE);

    module_load_include('inc', 'islandora_utils', 'includes/util');
    $pid = str_replace("_", ":", $set->setSpec);
    $local_pid = conformant_pid($pid) ? $pid : str_replace("_", "-", $set->setSpec) . ':collection';
    if ($local_pid == 'islandora:root') {
      continue;
    }
    $cmodels = array(
      array(
        'dsid' => '',
        'name' => 'Islandora Remote Resource Content Model',
        'namespace' => $local_pid,
        'pid' => 'islandora:sp_remote_resource'
      ),
    );

    $parent_local_pid = $parent = get_parent_collections($set->setSpec)[0];
    if (empty($parent)) {
      // if we can't get at the RELS (blocked or other), skip...
      continue;
    }

    if ($parent != 'islandora:root') {
      echo "checking $parent";
      $conformant = array_pop(explode(':', $parent)) == 'collection';
      $parent_local_pid = $conformant ? $parent : str_replace(':', '-', $parent) . ':collection';
    }

    $raw_collections[] = array(
      'title' => $description->title,
      'description' => $description->description,
      'pid' => $local_pid,
      'parent' => $parent_local_pid,
      'cmodels' => $cmodels,
    );
  }

  if (empty($raw_collections)) {
    echo "no collections!!";
    return;
  }

  reset($raw_collections);
  $c = current($raw_collections);
  do {
    printf("evaluating %s\n", $c['parent']);
    if ($c['parent'] == 'islandora:root' || islandora_object_load($c['parent'])) {
      islandora_utils_ingest_collection($c['title'], $c['description'], $c['pid'], $c['parent'], $c['cmodels']);
      printf("would have ingested %s\n", $c['title']);
      $key = key($raw_collections);

      unset($raw_collections[$key]);
      $c = next($raw_collections);
      if (!$c) {
        $c = reset($raw_collections);
      }
    }
    else {
      printf("nested collection!!!");
    }
  } while (count($raw_collections) > 1);
}

function conformant_pid($pid) {
  return array_pop(explode(':', $pid)) == 'collection';
}

function get_parent_collections($setspec) {
  $proto = drush_get_option('oai_endpoint_protocol', 'http');
  $host = drush_get_option('oai_remote_host');

  $remote_pid = str_replace('_', ':', $setspec);
  $rels_url = sprintf("%s://%s/islandora/object/%s/datastream/RELS-EXT/view", $proto, $host, $remote_pid);
  $rels = simplexml_load_string(file_get_contents($rels_url));
  if ($rels == FALSE) {
    return array();
  }

  $rels->registerXPathNamespace('fedora', 'info:fedora/fedora-system:def/relations-external#');
  $rels->registerXPathNamespace('rdf', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
  $parents = $rels->xpath('//fedora:isMemberOfCollection/@rdf:resource');
  $parent_pids = array();

  foreach ($parents as $parent) {
    $trimmed_parent = substr($parent['resource'], 12);
    $parent_pids[] = $trimmed_parent;
  }
  return $parent_pids;
}

function check_protocol($proto) {
  if (!in_array(strtolower($proto), array('http', 'https'))) {
    drush_die("Illegal value $proto given for oai_endpoint_protocol; must be one of [http|https]", 'error');
  }
}
