Commerce UPS
=================

Provides UPS shipping functionality for Drupal Commerce, based heavily on the 
work done by [Commerce Fedex](https://github.com/bmcclure/drupal-commerce_fedex).

## Development Setup

1. Clone this repository in to Drupal modules folder.

2. Manually get dependencies to installed Drupal.

`composer require gabrielbull/ups-api:~0.7`

3. Enable module.

4. Go to /admin/commerce/config/shipping-methods/add:
  - Select 'UPS' as the Plugin
  - Enter the UPS API details
  - Select a default package type
  - Select all the shipping services that should be enabled
  - Fill out any of the optional configs and save configuration
