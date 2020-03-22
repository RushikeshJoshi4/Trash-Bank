var Migrations = artifacts.require("./SellingContract.sol");

module.exports = function(deployer) {
  deployer.deploy(Migrations);
};
