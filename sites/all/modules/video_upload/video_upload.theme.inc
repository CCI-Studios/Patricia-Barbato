<?php


/**
 * @file video_upload.theme.inc
 * Theme functions for the Video Upload module.
 */

/**
 * Theme a video
 */
function theme_video_upload_video($variables) {
  $yt_id = $variables['yt_id'];
  $width = $variables['width'];
  $height = $variables['height'];
  $params = $variables['params'];
  $attributes = $variables['attributes'];
  $display = $variables['display'];

  if (!$yt_id) {
    // Output the video image instead.
    $attributes = array(
      'alt' => t('Video processing'),
      'title' => t('Video processing'),
    );
    return theme('video_upload_missing_image', array('style' => 'full', 'width' => $width, 'height' => $height, 'attributes' => $attributes));
  }

  // @todo: Abstract out YouTube.
  video_upload_initialize_provider();
  $video_url = VIDEO_UPLOAD_YOUTUBE_EMBED_URL . '/' . $yt_id;

  if (!$display['related_videos']) {
    // Related videos.
    $video_url .= '&rel=0';
  }
  if ($display['autoplay']) {
    // Autoplay.
    $video_url .= '&autoplay=1';
  }

  $default_params = array(
    'movie' => $video_url,
    'wmode' => 'transparent',
    'bgcolor' => '#FFFFFF',
  );

  if ($display['fullscreen']) {
    // Fullscreen.
    $video_url .= '&fs=1';
    $default_params['allowFullScreen'] = 'true';
  }

  $all_params = array_merge($default_params, $params);
  $parameters = '';
  foreach ($all_params as $p_name => $p_value) {
    $parameters .= '
    <param name="' . $p_name . '" value="' . $p_value .'" />
';
  }

  static $v = 1;
  $default_attributes = array(
    'type' => 'application/x-shockwave-flash',
    'height' => $height,
    'width' => $width,
    'data' => $video_url,
    'id' => 'videoEmbed' . $v,
    'allowFullScreen' => TRUE,
  );
  $v ++;

  $all_attributes = array_merge($default_attributes, $attributes);

  $embed_attributes = array(
    'src' => $all_attributes['data'],
    'type' => $all_attributes['type'],
    'wmode' => 'transparent',
    'width' => $all_attributes['width'],
    'height' => $all_attributes['height'],
    'allowFullScreen' => $all_attributes['allowFullScreen'],
  );
  $embed = '<embed ' . drupal_attributes($embed_attributes) . '></embed>';

  return '
  <object ' . drupal_attributes($all_attributes) . '>'
    . $parameters . $embed . '</object>';
}

/**
 * Theme a thumbnail.
 * @param array $display
 *   Widget display settings
 * @param array $item
 * @param string $path
 *   Path to the object
 * @todo offer random image (YT provides 3)
 */
function theme_video_upload_video_thumb($variables) {
  $item = $variables['item'];
  $display = $variables['display'];
  $path = $variables['path'];
  $width = $display['thumb_width'];
  $height = $display['thumb_height'];

  if (!isset($item['video_status'])) {
    return;
  }

  if (!$item['video_id']) {
    // Output the video image instead.
    $attributes = array(
      'alt' => t('Video processing'),
      'title' => t('Video processing'),
    );
    $image = theme('video_upload_missing_image', array('style' => 'thumbnail', 'width' => $width, 'height' => $height, 'attributes' => $attributes));

    if ($path) {
      $image = l($image, $path, array('html' => TRUE));
    }

    return $image;
  }
  else {
    if ($item['video_status'] == VIDEO_UPLOAD_STATUS_UNKNOWN) {
      // If the video isn't done processing, through a flag onto the end of the
      // url so the user's browser doesn't cache the processing thumbnail
      $flag = '?f=' . rand(0, 10);
      $attributes['title'] = t('Video processing');
    }

    // @todo: Abstract out YouTube.
    video_upload_initialize_provider();
    $attributes['path'] = VIDEO_UPLOAD_YOUTUBE_THUMB_URL . '/' . $item['video_id'] . '/1.jpg' . $flag;
    $attributes['alt'] = t('YouTube Thumbnail');
  }

  if ($width) {
    $attributes['width'] = $width;
  }
  if ($height) {
    $attributes['height'] = $height;
  }
  $img = theme('image', $attributes);
  if ($path) {
    $img = l($img, $path, array('html' => TRUE));
  }
  return $img;
}

/**
 * Theme a specific video upload status message
 * @fixme
 *   This function should be integrated with theme_video_upload_status_text(),
 *   as they do nearly the same thing in slightly different ways
 */
function theme_video_upload_video_status_message($item, $message = NULL) {
  if (!$message) {
    $message = t('Video hasn\'t finished processing yet');
  }
  return '<div class="messages">' . $message . '</div>';
}

/**
 * Theme the video upload admin form into a table/grid
 */
function theme_video_upload_admin_video_form($form) {
  $output = '';

  // Add css for this table.
  drupal_add_css(drupal_get_path('module', 'video_upload') . '/video_upload_admin.css');

  // Overview table:
  $select_all = isset($form['videos']) ? theme('table_select_header_cell') : array();
  $header = array($select_all, t('Title'), t('Id'), t('Node'), t('Field Info'), t('Status'));

  // Options
  $output .= drupal_render($form['options']);

  if (isset($form['id']) && is_array($form['id'])) {
    foreach (element_children($form['id']) as $key) {
      $row = array();
      // checkbox
      $row[] = drupal_render($form['videos'][$key]);
      // video title
      $row[] = drupal_render($form['title'][$key]);
      // video id
      $row[] = drupal_render($form['id'][$key]);
      // link to node, if available
      $row[] = $form['node'][$key] ? drupal_render($form['node'][$key]) : t('orphaned');
      // field info, if available
      $row[] = $form['field_name'][$key] ? drupal_render($form['field_name'][$key]) : t('n/a');
      // video status
      $row[] = drupal_render($form['status'][$key]);
      // highlight orphaned videos
      $class =  $form['node'][$key] ? '' : 'video-upload-orphaned';
      $rows[] = array('data' => $row, 'class' => $class);
    }
  }
  else  {
    $rows[] = array(array('data' => t('No videos have been uploaded to this account'), 'colspan' => '6'));
  }

  $output .= theme('table', $header, $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return '<div class="video-upload-admin">' . $output . '</div>';

}

/**
 * Theme the video status text
 * @param mixed $status
 *   A status object or integer
 */
function theme_video_upload_status_text($variables) {
  $status = $variables['status'];
  if (is_object($status)) {
    $stat = $status->status;
  }
  else {
    $stat = $status;
  }

  $class = 'video-upload-status';
  switch ($stat) {
    case VIDEO_UPLOAD_STATUS_ORPHANED :
      // @todo: abstract out YouTube
      $status_text = t('Video removed from YouTube');
      $class .= ' video-upload-status-bad';
      break;
    case VIDEO_UPLOAD_STATUS_BAD :
      $status_text = $status->message ? $status->message : t('Unknown Error');
      $class .= ' video-upload-status-bad';
      break;
    case VIDEO_UPLOAD_STATUS_UPLOAD_PENDING:
      $status_text = t('Video is being processed.');
      $class .= ' video-upload-processing';
      break;
    case VIDEO_UPLOAD_STATUS_OK :
      $status_text = t('OK');
      $class .= ' video-upload-status-ok';
      break;
    case VIDEO_UPLOAD_STATUS_OK_SYNCED :
      $status_text = t('Video up-to-date');
      $class .= ' video-upload-status-ok-synced';
      break;
    case VIDEO_UPLOAD_STATUS_DELETE :
      $status_text = t('Video is bad and queued for deletion');
      $class .= ' video-upload-status-delete';
      break;
    case VIDEO_UPLOAD_STATUS_UNKNOWN :
    default :
      $status_text = t('Video status is unknown. This most likely means it has not yet been processed.');
      $class .= ' video-upload-status-unknown';
  }

  $attributes = array(
    'class' => $class,
  );

  return '<span' . drupal_attributes($attributes) . '>' . $status_text . '</span>';
}

/**
 * Theme missing thumbnail image.
 *
 * @param string $style
 *   Thumbnail or full
 * @param integer $width
 * @param integer $height
 * @param array $attributes
 *   html attributes for inside the image tag
 */
function theme_video_upload_missing_image($variables) {
  $style = $variables['style'];
  $width = $variables['width'];
  $height = $variables['height'];
  $attributes = $variables['attributes'];

  $file = $style == 'full' ? 'video.png' : 'video-thumbnail.png';
  $image = drupal_get_path('module', 'video_upload') . '/images/' . $file;

  $defaults = array(
    'width' => $width,
    'height' => $height,
  );

  $attributes = array_merge($defaults, $attributes);
  $alt = isset($attributes['alt']) ? $attributes['alt'] : t('Video');
  $title = isset($attributes['title']) ? $attributes['title'] : '';
  $output = theme('image', array('path' => $image, 'alt' => $alt, 'title' => $title, 'attributes' => $attributes, 'getsize' => FALSE));  
  return $output;
}

/**
 * Default video upload field formatter.
 */
function theme_field_formatter_video_upload($variables) {
  $item = $variables['item'];
  $display = $variables['display'];
  if (!$item['fid']) {
    return;
  }

  if (!$item['video_id']) {
    drupal_set_message(theme('video_upload_status_text', array('status' => $item['video_status'])));
  }
  return theme('video_upload_video', array('yt_id' => $item['video_id'], 'width' => $display['default_width'], 'height' => $display['default_height'], 'display' => $display));
}

/**
 * Small video upload field formatter.
 */
function theme_field_formatter_video_upload_small($variables) {
  $item = $variables['item'];
  $display = $variables['display'];
  if (!$item['fid']) {
    return;
  }

  return theme('video_upload_video', array('yt_id' => $item['video_id'], 'width' => $display['small_width'],'height' =>  $display['small_height'], 'display' => $display));
}

/**
 * Thumbnail video upload field formatter.
 */
function theme_field_formatter_video_upload_thumb($variables) {
  $item = $variables['item'];
  $display = $variables['display'];
  if (!$item['fid']) {
    return;
  }

  return theme('video_upload_video_thumb', array('display' => $display, 'item' => $item, 'path' => FALSE));
}

/**
 * Thumbnail linked to node video upload field formatter.
 */
function theme_field_formatter_video_upload_thumb_link($variables) {
  $item = $variables['item'];
  $display = $variables['display'];
  if (!$item['fid']) {
    return;
  }

  $path = $variables['path'];
  return theme('video_upload_video_thumb', array('display' => $display, 'item' => $item, 'path' => $path));
}

/**
 * @} End defgroup "CCK Formatter theme functions."
 */
