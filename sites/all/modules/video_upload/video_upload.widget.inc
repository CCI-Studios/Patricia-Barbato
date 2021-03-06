<?php


/**
 * @file
 * Video_Upload widget hooks and callbacks.
 *
 * This file is loaded by fileifeld.module and the callbacks it contains are
 * invoked by filefield.module. This happens because video_upload delegates it's
 * CCK hooks and callbacks to filefield in video_upload.module.
 */

/**
 * Widget settings form.
 */
function _video_upload_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
  video_upload_initialize_provider();
  $form = module_invoke('file', 'field_widget_settings_form', $field, $instance);

  // Maintenance.
  $form['auto_delete_rejected_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto-delete rejected videos from account'),
    '#description' => t('When videos are rejected, use cron to automatically remove them from the video provider.'),
    '#default_value' => $settings['auto_delete_rejected_videos'],
  );
  $form['remove_deleted_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove videos from provider if node is deleted, or video is changed'),
    '#description' => t("When a node is deleted, or a video is changed/updated the original video will be removed from the provider. If left un-checked, videos will remain on the 3rd-party provider's site."),
    '#default_value' => $settings['remove_deleted_videos'],
  );

  // Width and height.
  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display Options'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );
  $form['display']['default_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Default/Large Width'),
    '#required' => TRUE,
    '#default_value' => $settings['display']['default_width'],
    '#description' => t('The width for the default (large) player'),
  );
  $form['display']['default_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Default/Large Height'),
    '#required' => TRUE,
    '#default_value' => $settings['display']['default_height'],
    '#description' => t('The height for the default (large) player'),
  );
  $form['display']['small_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Small Width'),
    '#required' => TRUE,
    '#default_value' => $settings['display']['small_width'],
    '#description' => t('The width for the small player'),
  );
  $form['display']['small_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Small Height'),
    '#required' => TRUE,
    '#default_value' => $settings['display']['small_height'],
    '#description' => t('The height for the small player'),
  );
  $form['display']['thumb_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Thumbnail Width'),
    '#default_value' => $settings['display']['thumb_width'],
  );
  $form['display']['thumb_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Thumbnail Height'),
    '#default_value' => $settings['display']['thumb_height'],
  );
  // @TODO abstract out youtube. The options below are (mostly)
  //       YouTube-specific, and should be added via a provider hook.
  $form['display']['related_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show related videos'),
    '#default_value' => $settings['display']['related_videos'],
    '#description' => t('Show related videos in the player after the video has finished.'),
  );
  $form['display']['autoplay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto-play video on page load'),
    '#default_value' => $settings['display']['autoplay'],
  );
  $form['display']['fullscreen'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Fullscreen'),
    '#description' => t('Show a <em>fullscreen</em> button on the embedded player.'),
    '#default_value' => $settings['display']['fullscreen'],
  );

  // Default title pattern.
  $form['default_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Video Title'),
    '#default_value' => $settings['default_title'],
    '#size' => 64,
    '#maxlength' => 255,
    '#description' => t('Since the videos are initially uploaded prior to node creation, they must contain a default title. This title will only be visible on the provider site, and will be updated once the node is saved, and the video has been processed.'),
  );
  if (module_exists('token')) {
    $form['default_title']['#description'] .= ' '. t('<br />The following tokens can be used in the default title');
    $form['default_title']['#suffix'] = theme('fieldset', array('#children' => theme('token_help', 'user'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Default title synchronization.
  $form['default_title_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Title Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider, these options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'), 
      VIDEO_UPLOAD_SYNC_NONE => t('Don\'t Synchronize (keep default)'),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append node title to default'),
    ),
    '#default_value' => $settings['default_title_sync'],
    '#required' => TRUE,
  );

  // Default description.
  $form['default_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Default video description'),
    '#default_value' => $settings['default_description'],
    '#description' => t('Since the videos are initially uploaded prior to node creation, they can contain a default description. This description will only be visible on the provider site, and will be updated with the node teaser once the node is saved and the video has been processed.'),
    '#element_validate' => array('_video_upload_widget_validate_strip_tags'),
  );
  if (module_exists('token')) {
    $form['default_description']['#description'] .= t('<br />The following tokens can be used in the default description');
    $form['default_description']['#suffix'] = theme('fieldset', array('#children' => theme('token_help'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Default description synchronization.
  $form['default_description_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Description Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider, these options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'),
      VIDEO_UPLOAD_SYNC_NONE => t('Don\'t Synchronize (keep default)'),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append Description to default')
    ),
    '#default_value' => $settings['default_description_sync'],
    '#required' => TRUE,
  );

  // Keywords.
  // @todo - some input validation
  $form['default_keywords'] = array(
    '#type' => 'textfield',
    '#title' => t('Default keywords'),
    '#required' => TRUE,
    '#description' => t('Comma-separated list of keywords (no white space allowed in keywords).<br /><br />Since the videos are initially uploaded prior to node creation, they must contain a default set of keywords. These keywords will only be visible on the provider site, and will be updated with taxonomy terms (if in use on this content type) once the node is saved and the video has been processed.'),
    '#default_value' => $settings['default_keywords'],
    '#element_validate' => array('_video_upload_widget_validate_default_keywords'),
  );
  // default keyword synchronization
  $form['default_keyword_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Keyword Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider site. These options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'),
      VIDEO_UPLOAD_SYNC_NONE => t("Don't Synchronize (keep default)"),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append Taxonomy (if applicable) to default keywords')
    ),
    '#default_value' => $settings['default_keyword_sync'],
    '#required' => TRUE,
  );

  // Developer tags.
  $form['developer_tags'] = array(
    '#type' => 'textfield',
    '#title' => t('Developer Tags'),
    '#default_value' => $settings['developer_tags'],
    '#description' => t('Videos uploaded to the video provider can have hidden tags associated with them for later organization and tracking. Separate tags with a comma.'),
  );
  if (module_exists('token')) {
    $form['developer_tags']['#description'] .= t('<br />The following tokens can be used in <em>Developer Tags</em>');
    $form['developer_tags']['#suffix'] = theme('fieldset', array('#children' => theme('token_help', 'user'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Video category (this is a fixed list from YouTube).
  // @todo abstract out YouTube
  $form['video_category'] = array(
    '#type' => 'select',
    '#title' => t('Video Category'),
    '#options' => _video_upload_youtube_available_categories(),
    '#default_value' => $settings['video_category'],
    '#required' => TRUE,
    '#description' => t('The video provider requires that videos be placed into one of these categories'),
  );

  return $form;
}

/**
 * Validate default keywords setting.
 */
function _video_upload_widget_validate_default_keywords($element, &$form_state) {
  if (isset($form_state['values']['default_keywords']) && $form_state['values']['default_keywords']) {
    // @todo abstract out YouTube. This should simply call a provider callback
    //       instead of enforcing a YouTube-specific keyword format.
    // Keywords can't have spaces in them.
    $check = explode(',', $form_state['values']['default_keywords']);
    foreach ($check as $keyword) {
      $keyword = trim($keyword);
      if (strpos($keyword, ' ') !== FALSE) {
        form_error($element, t("No spaces are allowed in keywords."));
        return;
      }
      $keywords[] = $keyword;
    }
    form_set_value($element, implode(',', $keywords), $form_state);
  }
}

/**
 * Strip all tags out of description fields.
 */
function _video_upload_widget_validate_strip_tags($element, &$form_state) {
  if (isset($form_state['values'][$element['#name']])) {
    $field = $form_state['values'][$element['#name']];
    form_set_value($element, strip_tags($field), $form_state);
  }
}