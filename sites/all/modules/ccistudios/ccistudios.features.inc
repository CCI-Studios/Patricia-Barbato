<?php
/**
 * @file
 * ccistudios.features.inc
 */

/**
 * Implements hook_views_api().
 */
function ccistudios_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function ccistudios_node_info() {
  $items = array(
    'media_video_list' => array(
      'name' => t('Media Video'),
      'base' => 'node_content',
      'description' => t('Video on the media page'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'page' => array(
      'name' => t('Basic page'),
      'base' => 'node_content',
      'description' => t('Use <em>basic pages</em> for your static content, such as an \'About us\' page.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
    'text_rotator' => array(
      'name' => t('About Text Slideshow'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
