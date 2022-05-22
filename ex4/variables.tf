variable "network_interface_id" {
  type = string
  default = "network_id_from_aws"
}

variable "ami" {
    type = string
    default = "ami-0022f774911c1d690"
}

variable "first_instance_type" {
    type = string
    default = "m4.large"
}
