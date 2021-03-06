<?php


/**
 * @file video_upload.admin.inc
 */

/**
 * Callback function for admin/settings/video-upload
 * @return
 *   Returns an array defining the admin settings form
 */
function _video_upload_admin_settings_form() {
  // @todo: Abstract out YouTube.
  $form['youtube'] = array(
    '#type' => 'fieldset',
    '#title' => t('YouTube API Settings'),
  );
  $form['youtube']['video_upload_youtube_developer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Developer Key'),
    '#description' => t('A YouTube <a href="!url">Developer Key</a> is required', array('!url' => url('http://code.google.com/apis/youtube/dashboard/'))),
    '#default_value' => variable_get('video_upload_youtube_developer_key', FALSE),
    '#required' => TRUE,
  );
  $form['youtube']['video_upload_youtube_username'] = array(
    '#type' => 'textfield',
    '#title' => t('YouTube Username'),
    '#default_value' => variable_get('video_upload_youtube_username', FALSE),
    '#required' => TRUE,
  );
  $form['youtube']['video_upload_youtube_password'] = array(
    '#type' => 'textfield',
    '#title' => t('YouTube Account Password'),
    '#default_value' => variable_get('video_upload_youtube_password', FALSE),
    '#required' => TRUE,
  );
  $form['video_upload_cron_queue_time'] = array(
    '#type' => 'textfield',
    '#title' => t('Max upload time'),
    '#description' => t('Maximum time, in seconds, cron should devote to uploading videos.'),
    '#required' => TRUE,
    '#default_value' => variable_get('video_upload_cron_queue_time', 600),
  );

  return system_settings_form($form);
}

/**
 * Menu callback: administer video
 */
function video_upload_admin_video() {
  if (isset($_POST['operation']) && $_POST['operation'] == 'delete' && $_POST['videos']) {
    return drupal_get_form('video_upload_admin_video_delete_confirm');
  }
  $output = drupal_get_form('video_upload_admin_video_form');
  return $output;
}

/**
 * Video deletion confirmation
 */
function video_upload_admin_video_delete_confirm() {
  $edit = $_POST;

  $form['videos'] = array(
    '#prefix' => '<ul>', 
    '#suffix' => '</ul>', 
    '#tree' => TRUE
  );

  // array_filter returns only elements with TRUE values
  foreach (array_filter($edit['videos']) as $vid => $value) {
    $form['videos'][$vid] = array(
      '#type' => 'hidden',
      '#value' => 'delete',
      '#prefix' => '<li>',
      '#suffix' => check_plain($edit['vtitle-' . $vid]) . '</li>' . "\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');

  return confirm_form(
    $form,
    t('Are you sure you want to delete these videos?'),
    'admin/content/video-upload', t('This action cannot be undone.'),
    t('Delete all'), t('Cancel')
  );
}

/**
 * Delete videos after confirmation.
 */
function video_upload_admin_video_delete_confirm_submit($form_id, $form_values) {
  if ($form_values['values']['confirm']) {
    // @todo: Abstract out YouTube.
    $connection = video_upload_connect(TRUE);
    foreach ($form_values['values']['videos'] as $vid => $value) {
      $video = _video_upload_get_video_object_by_id($vid, $connection);
      if ($video) {
        _video_upload_delete_video_remote($connection, $video);
        drupal_set_message(t('Video %id has been removed from YouTube', array('%id' => $vid)));
      }
    }
    drupal_set_message(t('The videos have been deleted'));
  }

  return 'admin/content/video-upload';
}

/**
 * Process video admin form
 */
function video_upload_admin_video_form_submit($form_id, $form_values) {
}

/**
 * Video administration form

 * @todo Replace these queries with calls to the cck api, or similar, since
 * they currently break once a field has been reused in another type.
 */
function video_upload_admin_video_form() {
  // @todo: Abstract out YouTube.
  // Get YouTube connection.
  $connection = video_upload_connect(TRUE);

  // Get feed of all videos.
  $feed = _video_upload_gdata_get_feed(VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED, $connection);

  // Get list of all videos.
  $all_videos = array();
  $fields = field_info_fields(NULL, NULL, 'video_upload');
  foreach ($fields as $key => $field) {
    if ($field['type'] != 'video_upload') {
      unset($fields[$key]);
      continue;
    }
    $result = _video_upload_query_videos($field);
    foreach ($result as $entity) {
      // add field_name
      $entity->field_name = $key;
      $language = field_language($entity->object, $entity, $field['field_name']);
      foreach ($entity->{$field['field_name']}[$language] as $delta => $video) {
        $all_videos[$video['video_id']] = $video;
      }
    }
  }

  $video_ids = array_keys($all_videos);

  // The big feed list will be a combination of gdata objects and node
  // objects (in the event of orphaned nodes).
  $big_feed_list = $video_ids_youtube = array();

  // Now, with a list of ids, loop through the feed looking for
  // isolated/stranded videos that have a developer tag set by one of
  // our fields.
  if ($feed) {
    foreach ($feed as $video) {
      $id = $video->getVideoId();

      // add id to the youtube ids (this is to find orphaned nodes later on)
      $video_ids_youtube[] = $id;

      $big_feed_list[] = $video;
    }
  }

  // Now, loop through drupal videos to find video ids not on youtube
  foreach ($all_videos as $id => $video) {
    if (!in_array($id, $video_ids_youtube)) {
      $node = node_load($video->nid);
      $video->title = $node->title;
      $video->link = l($node->title, 'node/' . $node->nid);
      $big_feed_list[] = $video;
    }
  }

  // loop through combined list of feed video and orphaned video
  foreach ($big_feed_list as $video) {
    // @todo abstract out youtube
    if (!_video_upload_is_zend_object($video)) {
      // Due to the strict OO nature of the Zend client library, we must use
      // this function instead of a simple boolean check
      $id = $video->ytid;
    }
    else {
      $id = $video->getVideoId();

      // set video title
      $video->title = check_plain($video->getVideoTitle());
      // set link
      $video->link = l($id, video_upload_link_youtube($id));
    }

    $form['id'][$id] = array(
      '#value' => $video->link,
    );

    $form['title'][$id] = array(
      '#value' => $video->title,
    );

    $form['vtitle'][$id] = array(
      '#name' => 'vtitle-' . $id,
      '#type' => 'hidden',
      '#value' => $video->title,
    );

    $node = FALSE;
    if (in_array($id, $video_ids)) {
      // this video is associated with a node, so we can fetch the node to
      // determine the status
      $nid = $all_videos[$id]->nid;

      $node = node_load($nid);

      $form['node'][$id] = array('#value' => l($nid, 'node/' . $nid));

      $field_name = $all_videos[$id]->field_name;
      $form['field_name'][$id] = array('#value' => l($field_name, 'admin/content/node-type/' . $node->type . '/fields'));
    }
    else {
      // Orphaned video, so we can operate on it. Videos associated with nodes
      // must be operated on through the node system.
      $videos[$id] = '';

      $form['node'][$key] = FALSE;
      $form['field_name'][$key] = FALSE;
    }

    // if the node has the status, display that
    $status = VIDEO_UPLOAD_STATUS_UNKNOWN;
    if ($node) {
      $status = $id ? $node->{$all_videos[$id]->field_name}[0]['status'] : VIDEO_UPLOAD_STATUS_UPLOAD_PENDING;
    }

    if (_video_upload_is_zend_object($video) && $status < VIDEO_UPLOAD_STATUS_OK) {
      // find status from YouTube since it either hasn't been checked,
      // or this is an orphaned node
      $status = _video_upload_youtube_get_status_by_id($id, $connection);
    }
    elseif ($status != VIDEO_UPLOAD_STATUS_UPLOAD_PENDING && !_video_upload_is_zend_object($video)) {
      $status = VIDEO_UPLOAD_STATUS_ORPHANED;
    }


    $status_text = theme('video_upload_status_text', $status);
    $form['status'][$id] = array('#value' => $status_text);
  }

  // This constructs the checkboxes, and in fact, the only interactivity
  // currently available. If there aren't any orphaned videos, there's nothing
  // to do.
  if (!empty($videos)) {
    $form['videos'] = array(
      '#type' => 'checkboxes',
      '#options' => $videos,
    );

    $form['options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Video management options'),
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );
    $options = array('delete' => t('Delete selected videos'));
    $form['options']['operation'] = array(
      '#type' => 'select', 
      '#options' => $options,  
      '#default_value' => 'approve');
    $form['options']['submit'] = array(
      '#type' => 'submit', 
      '#value' => t('Submit'));
  }

  $form['pager'] = array('#value' => theme('pager', array('parameters' => array('limit' => 50))));

  return $form;
}

/**
 * Upload all queued videos to the provider.
 *
 * @param array $fields
 *   CCK field definitions.
 *
 * @return integer
 *   Number of videos uploaded.
 */
function _video_upload_upload_all($fields) {
  $uploaded = 0;
  foreach ($fields as $name => $field) {
    $result = _video_upload_query_videos($field, VIDEO_UPLOAD_STATUS_UPLOAD_PENDING);
    foreach ($result as $entity) {
      // Attach the widget/instance definition.
      $field = $entity->field = field_info_instance($entity->object, $field['field_name'], $entity->bundle);

      // Upload video.
      if ($count = video_upload_upload($field, $entity)) {
        $uploaded += $count;
      }
    }
  }
  return $uploaded;
}

/**
 * Cycle through all records in a table with status of 0 and attempt
 * to verify them on YouTube. Once verified, also attempt to update
 * title/tags on YouTube.
 *
 * @param $fields
 *   CCK configuration for video upload fields.
 */
function _video_upload_verify_all($fields) {
  // @TODO Abstract out youtube.
  $video_ids = $videos = array();
  foreach ($fields as $name => $field) {
    $result = _video_upload_query_videos($field, VIDEO_UPLOAD_STATUS_UNKNOWN);
    foreach ($result as $entity) {
      $entity->field = field_info_instance($entity->object, $field['field_name'], $entity->bundle);
      $language = field_language($entity->object, $entity, $field['field_name']);
      foreach ($entity->{$field['field_name']}[$language] as $delta => $video) {
        $video_ids[] = $video['video_id'];

        // Attach relevant delta for saving purposes.
        $entity->delta = $delta;

        $videos[$video['video_id']] = $entity;
      }
    }
  }

  // Establish provider connection.
  $connection = video_upload_connect(TRUE);

  // Get a feed of all the user's videos (!) 
  // @fixme
  //   Unfortunately, this is currently the only way to check on
  //   videos that are not yet published.  A much more efficient and
  //   sane method would be to only look up the videos on which we
  //   don't have a status.
  $feed = _video_upload_gdata_get_feed(VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED, $connection);

  if ($feed) {
    foreach ($feed as $video) {
      if (!in_array($video->getVideoId(), $video_ids)) {
        // This isn't in the list of videos we currently care to check.
        continue;
      }

      $status = _video_upload_gdata_get_video_status($video);

      if ($status->status === FALSE) {
        // The video is bad, the node should be deleted, or unpublished, or
        // perhaps the user should be notified Setting status to -1 queues
        // the video for deletion.
        $entity = $videos[$video->getVideoId()];
	$language = field_language($entity->object, $entity, $entity->field['field_name']);
        $entity->{$entity->field['field_name']}[$language][$entity->delta]['video_status'] = VIDEO_UPLOAD_STATUS_BAD;
        _video_upload_update_video($entity);
      }
      elseif ($status->status === TRUE) {
        // The video is good, update the table.
        $entity = $videos[$video->getVideoId()];
	$language = field_language($entity->object, $entity, $entity->field['field_name']);
        $local_video = $entity->{$entity->field['field_name']}[$language][$entity->delta];

        // Update YouTube if applicable.
        _video_upload_update_video_remote($connection, $video, $local_video, $entity);

        // The video is good, update the table.
        $entity->{$entity->field['field_name']}[$language][$entity->delta] = $local_video;
        _video_upload_update_video($entity);

        // @TODO Notify the author, or expose this to trigger/action functionality.
      }
      else {
        // Nothing found on the video, wait until next time.
      }
    } 
  }
}

/**
 * Cycle through all records in a table with status of 1 and update
 * info on YouTube.
 * @param $fields
 *   CCK configuration for video upload fields.
 */
function _video_upload_update_all_videos($fields) {
  // @todo Abstract out YouTube.
  $video_ids = $videos = array();
  foreach ($fields as $name => $field) {
    $result = _video_upload_query_videos($config, VIDEO_UPLOAD_STATUS_OK);

    foreach ($result as $entity) {
      $entity->field = field_info_instance($entity->object, $field['field_name'], $entity->bundle);
      $language = field_language($entity->object, $entity, $entity->field['field_name']);
      foreach ($entity->{$field['field_name']}[$language] as $delta => $video) {
        $video_ids[] = $video['video_id'];

        // Attach relevant delta for saving purposes.
        $entity->delta = $delta;
        $videos[$video['video_id']] = $entity;
      }
    }
  }

  // Establish provider connection.
  $connection = _video_upload_youtube(TRUE);

  // Get a feed of all the user's videos (!). 
  // @fixme unfortunately, this is currently the only way to check on videos
  //        that are not yet published.  A much more efficient and sane
  //        method would be to only look up the videos on which we don't
  //        have a status
  $feed = _video_upload_gdata_get_feed(VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED, $connection);

  if ($feed) {
    foreach ($feed as $video) {
      if (!in_array($video->getVideoId(), $video_ids)) {
        continue;
      }
      // Load the node in question.
      $local_video = $videos[$video->getVideoId()];
      $entity = $videos[$video->getVideoId()];
      $language = field_language($entity->object, $entity, $entity->field['field_name']);
      $local_video = $entity->{$entity->field['field_name']}[$language][$entity->delta];
      _video_upload_update_video_remote($connection, $video, $local_video, $entity);

      // The video may have been updated, thus the status may have changed.
      _video_upload_update_video($local_video);
    }
  }
}

/**
 * Cycle through all records in a table with status of -1 and delete
 * them if the field is configured to do so.
 *
 * @param array $fields
 *   CCK configuration for video upload fields.
 */
function _video_upload_delete_rejected_videos($fields) {
  // @todo Abstract out YouTube.
  $video_ids = $videos = array();
  foreach ($fields as $name => $field) {
    // @TODO this setting needs to be per-field, not per instance.
    if (!$field['widget']['auto_delete_rejected_videos'] && !$config['widget']['remove_deleted_videos']) {
      continue;
    }
    $result = _video_upload_query_videos($config, VIDEO_UPLOAD_STATUS_DELETE);
    foreach ($result as $video) {
      $video_ids[] = $video->video_id;
      $videos[$video->video_id] = $video;
    }
  }

  // Establish provider connection.
  $connection = video_upload_connect(TRUE);

  // Get a feed of all the user's videos (!).
  // @FIXME
  //   Unfortunately, this is currently the only way to check on videos that
  //   are not yet published.  A much more efficient and sane method would be
  //   to only look up the videos on which we don't have a status.
  $feed = _video_upload_gdata_get_feed(VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED, $connection);

  if ($feed) {
    foreach ($feed as $video) {
      if (!in_array($video->getVideoId(), $video_ids)) {
        continue;
      }

      // Delete the video from the provider.
      _video_upload_delete_video_remote($connection, $video);

      // Remove record from local table.
      // @TODO this is provider-specific.
      $local_video = $videos[$video->getVideoId()];
      video_upload_delete_local($local_video);
    }
  }
}

/**
 * Query to find all videos for a given field.
 *
 * @param array $field
 *   Field definition array.
 * @param string $video_status
 *   Video status
 * @return
 *   Array of fully loaded entity objects.
 */
function _video_upload_query_videos($field, $status = FALSE) {
  $filter = array();
  if ($status) {
    $filter = array(array('video_status', $status));
  }
  $results = field_attach_query($field['id'], $filter);
  $video_entities = array();
  foreach ($results as $object => $entities) {
    // Get details on the object type so those can be passed along
    // with the video.
    $object_info = entity_get_info($object);
    $id_field = $object_info['entity keys']['id'];

    // Load all matching entities.
    $loaded_entities = entity_load($object, array_keys($entities));

    foreach ($loaded_entities as $entity) {
      $entity->object = $object;
      $entity->bundle = $entities[$entity->$id_field]->type;
      $video_entities[] = $entity;
    }
  }
  return $video_entities;
}