<?php

/* hook_settings_form() */
function commerce_ups_settings_form($form, &$form_state) {
  $form['api'] = array(
    '#type' => 'fieldset',
    '#title' => t('UPS API credentials'),
    '#collapsible' => TRUE,
  );
    
    $form['api']['commerce_ups_account_id'] = array(
      '#type' => "textfield",
      '#title' => t("Account ID"),
      '#value' => variable_get('commerce_ups_account_id')
    );
    $form['api']['commerce_ups_user_id'] = array(
      '#type' => "textfield",
      '#title' => t("User ID"),
      '#value' => variable_get('commerce_ups_user_id')
    );
    $form['api']['commerce_ups_password'] = array(
      '#type' => "password",
      '#title' => t("Password"),
    );
    $form['api']['commerce_ups_access_key'] = array(
      '#type' => "textfield",
      '#title' => t("Access Key"),
      '#value' => variable_get('commerce_ups_access_key')
    );

  $form['origin'] = array(
    '#type' => "fieldset",
    '#title' => t("Origin Address"),
    '#collapsible' => true
  );
    
    // $store['name'] = variable_get('uc_store_name', NULL);
    // $store['owner'] = variable_get('uc_store_owner', NULL);
    // $store['email'] = variable_get('uc_store_email', NULL);
    // $store['email_from'] = variable_get('uc_store_email', NULL);
    // $store['phone'] = variable_get('uc_store_phone', NULL);
    // $store['fax'] = variable_get('uc_store_fax', NULL);
    // $store['street1'] = variable_get('uc_store_street1', NULL);
    // $store['street2'] = variable_get('uc_store_street2', NULL);
    // $store['city'] = variable_get('uc_store_city', NULL);
    // $store['zone'] = variable_get('uc_store_zone', NULL);
    // $store['postal_code'] = variable_get('uc_store_postal_code', NULL);
    // $store['country'] = variable_get('uc_store_country', 840);
    
    // $form['origin']
  
  $form['services'] = array(
    '#type' => 'fieldset',
    '#title' => t("Enable UPS Shipping Services"),
    '#collapsible' => TRUE
  );
  
    foreach(_commerce_ups_service_list() as $key => $service)
      $arrOpts[$key] = $service['title'];
    
    $form['services']['commerce_ups_services'] = array(
      '#type' => 'checkboxes',
      '#options' => $arrOpts,
      '#default_value' => variable_get("commerce_ups_services")
    );   
    
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/* hook_settings_form_validate() */
function commerce_ups_settings_form_validate($form, &$form_state) {

  $old_password = variable_get('commerce_ups_password', '');
  if (!$form_state['values']['commerce_ups_password']) {
    if ($old_password) {
      form_set_value($form['api']['commerce_ups_password'], $old_password, $form_state);
    }
    else {
      form_set_error('password', t('Password field is required.'));
    }
  }
}


/* hook_settings_submit() */
function commerce_ups_settings_form_submit($form, &$form_state) {
  $fields = array(
    'commerce_ups_account_id',
    'commerce_ups_user_id',
    'commerce_ups_password',
    'commerce_ups_access_key',
    'commerce_ups_services',
  );

  foreach ($fields as $key) {
    $value = $form_state['values'][$key];

    if (is_array($value) && isset($form_state['values']['array_filter'])) {
      $value = array_keys(array_filter($value));
    }
    variable_set($key, $value);
  }

  drupal_set_message(t('The configuration options have been saved.'));

  cache_clear_all();
  drupal_theme_rebuild();
}
