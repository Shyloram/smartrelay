#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

# Certificate files. certificate.pem.crt & private.pem.key must be downloaded
# from AWS, see README for details.
COMPONENT_EMBED_TXTFILES := certs/aws-root-ca.pem certs/certificate.pem.crt certs/private.pem.key

ifndef IDF_CI_BUILD
# Print an error if the certificate/key files are missing
$(COMPONENT_PATH)/certs/certificate.pem.crt $(COMPONENT_PATH)/certs/private.pem.key:
	@echo "Missing PEM file $@. This file identifies the ESP32 to AWS for the example, see README for details."
	exit 1
else  # IDF_CI_BUILD
# this case is for the internal Continuous Integration build which
# compiles all examples. Add some dummy certs so the example can                                                                                      
# compile (even though it won't work)
$(COMPONENT_PATH)/certs/certificate.pem.crt $(COMPONENT_PATH)/certs/private.pem.key:
	echo "Dummy certificate data for continuous integration" > $@
endif

COMPONENT_ADD_INCLUDEDIRS := include include/awsiot include/aplink
COMPONENT_SRCDIRS := . awsiot aplink 
