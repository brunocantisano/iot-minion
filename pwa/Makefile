# import deploy config
# You can change the default deploy config with `make cnf="deploy_special.env" release`
dpl ?= .env
include $(dpl)
export $(shell sed 's/=.*//' $(dpl))

# HELP
# This will output the help for each task
# thanks to https://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
.PHONY: help

help: ## This help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.DEFAULT_GOAL := help


# TASKS
# Build
build: ## Build the release container.
# nvm use v16.16.0
	npm run build

# Start
start: ## Run the application
	npm run start

sonar: ## Run sonar to analyze code
	sonar-scanner

test: ## Running unit tests
	npm run test

lint: ## Running lint sintax
	npm run lint

e2e: ## Execute the end-to-end tests
	npm run e2e

token: ## Generate firebase token
	npm install -g firebase-tools
	firebase login:ci

login: ## Deploy to firebase
	npm run firebase:login

deploy: login ## Deploy to firebase
	npm run firebase:deploy