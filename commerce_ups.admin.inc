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
    '#title' => t("Ship From Address"),
    '#collapsible' => true
  );
    $form['origin']['commerce_ups_company_name'] = array(
      '#type' => "textfield",
      '#title' => t("Company Name"),
      '#value' => variable_get('commerce_ups_company_name')
    );
    $form['origin']['commerce_ups_address_line_1'] = array(
      '#type' => 'textfield',
      '#title' => t("Address"),
      '#value' => variable_get('commerce_ups_address_line_1')
    );
    $form['origin']['commerce_ups_address_line_2'] = array(
      '#type' => 'textfield',
      '#title' => t("Address (Additional)"),
      '#value' => variable_get('commerce_ups_address_line_2')
    );
    $form['origin']['commerce_ups_city'] = array(
      '#type' => 'textfield',
      '#title' => t("City"),
      '#value' => variable_get('commerce_ups_city')
    );
    $form['origin']['commerce_ups_state'] = array(
      '#type' => 'select',
      '#title' => t("State"),
      '#value' => variable_get('commerce_ups_state'),
      '#options' => array(
      	'' => t('Please Select'),
      	'AL' => t('Alabama'),
      	'AK' => t('Alaska'),
      	'AZ' => t('Arizona'),
      	'AR' => t('Arkansas'),
      	'CA' => t('California'),
      	'CO' => t('Colorado'),
      	'CT' => t('Connecticut'),
      	'DE' => t('Delaware'),
      	'DC' => t('District of Columbia'),
      	'FL' => t('Florida'),
      	'GA' => t('Georgia'),
      	'HI' => t('Hawaii'),
      	'ID' => t('Idaho'),
      	'IL' => t('Illinois'),
      	'IN' => t('Indiana'),
      	'IA' => t('Iowa'),
      	'KS' => t('Kansas'),
      	'KY' => t('Kentucky'),
      	'LA' => t('Louisiana'),
      	'ME' => t('Maine'),
      	'MD' => t('Maryland'),
      	'MA' => t('Massachusetts'),
      	'MI' => t('Michigan'),
      	'MN' => t('Minnesota'),
      	'MS' => t('Mississippi'),
      	'MO' => t('Missouri'),
      	'MY' => t('Montana'),
      	'NE' => t('Nebraska'),
      	'NV' => t('Nevada'),
      	'NH' => t('New Hampshire'),
      	'NJ' => t('New Jersey'),
      	'NM' => t('New Mexico'),
      	'NY' => t('New York'),
      	'NC' => t('North Carolina'),
      	'ND' => t('North Dakota'),
      	'OH' => t('Ohio'),
      	'OK' => t('Oklahoma'),
      	'OR' => t('Oregon'),
      	'PA' => t('Pennsylvania'),
      	'RI' => t('Rhode Island'),
      	'SC' => t('South Carolina'),
      	'SD' => t('South Dakota'),
      	'TN' => t('Tennessee'),
      	'TX' => t('Texas'),
      	'UT' => t('Utah'),
      	'VT' => t('Vermont'),
      	'VA' => t('Virginia'),
      	'WA' => t('Washington'),
      	'WV' => t('West Virginia'),
      	'WI' => t('Wisconsin'),
      	'WY' => t('Wyoming'),
      )
    );
    $form['origin']['commerce_ups_postal_code'] = array(
      '#type' => 'textfield',
      '#title' => t("Postal Code"),
      '#size' => 5,
      '#value' => variable_get('commerce_ups_postal_code')
    );
    $form['origin']['commerce_ups_country_code'] = array(
      '#type' => 'select',
      '#title' => t("Country"),
      '#value' => variable_get('commerce_ups_country_code'),
      '#options' => array(
      	'' => t('Please Select'),
      	'US' => t('United States of America'),
    )
  );

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
      '#default_value' => variable_get("commerce_ups_services", array())
    );   
    
    
  $form['packaging'] = array(
    '#type' => 'fieldset',
    '#title' => t("Enable UPS Packaging Types"),
    '#collapsible' => TRUE
  );

    $form['packaging']['commerce_ups_packaging'] = array(
      '#type' => 'checkboxes',
      '#options' => _commerce_ups_packaging_types(),
      '#default_value' => variable_get("commerce_ups_packaging", array())
    );   
    
 $form['commerce_ups_pick_up_schedule'] = array(
  "#type" => "select",
  "#title" => t("Pick-up Schedule"),
  "#options" => _commerce_ups_pickup_types(),
  "#value" => variable_get("commerce_ups_pick_up_schedule")
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
    'commerce_ups_company_name',
    'commerce_ups_address_line_1',
    'commerce_ups_address_line_2',
    'commerce_ups_city',
    'commerce_ups_state',
    'commerce_ups_postal_code',
    'commerce_ups_country_code',
    'commerce_ups_packaging',
    'commerce_ups_pick_up_schedule'
  );
  
  if(empty($form_state['input']['commerce_ups_password']))
    $form_state['input']['commerce_ups_password'] = $form_state['values']['commerce_ups_password'];

  foreach ($fields as $key) {
    $value = $form_state['input'][$key];
    variable_set($key, $value);
  }


  drupal_set_message(t('The configuration options have been saved.'));

  cache_clear_all();
  drupal_theme_rebuild();
}
