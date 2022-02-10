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
	npm install
	npm run build

# Start
start: npm run start ## Run the application

sonar: ## Run sonar to analyze code
	sonar-scanner

test: ## Running unit tests
	npm run test

lint: ## Running lint sintax
	npm run lint

e2e: ## Execute the end-to-end tests
	npm run e2e

firebase-token: ## Generate firebase token
	firebase login:ci
