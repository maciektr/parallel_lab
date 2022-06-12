terraform {
  required_providers {
    aws = {
      source = "hashicorp/aws"
    }
  }
}

provider "aws" {
  region = "us-east-1"  
}

resource "aws_instance" "mpr_instance_1" {
  ami           = var.ami
  instance_type = var.first_instance_type
  associate_public_ip_address = true
  key_name = "vockey"

  root_block_device {
    volume_size = 30 
  }

  tags = {
    Name = "mpr_inst_1"
    Cores = 2
  }
}

output "instance_1_ip" {
  description = "The public ip for ssh access"
  value       = aws_instance.mpr_instance_1.public_ip
}
