<?php

namespace Drupal\Tests\commerce_ups\Unit;

use Drupal\Tests\UnitTestCase;

/**
 * Class UPSUnitTestBase.
 *
 * @package Drupal\Tests\commerce_ups\Unit
 */
abstract class UPSUnitTestBase extends UnitTestCase {
  protected $configuration;

  /**
   * {@inheritdoc}
   */
  protected function setUp() {
    parent::setUp();
    $this->configuration = [
      'api_information' => [
        'access_key' => '123',
        'user_id' => '123',
        'password' => '123',
        'mode' => 'test',
      ],
      'rate_options' => [
        'rate_type' => TRUE,
      ],
    ];
  }

}
