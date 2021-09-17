/* @file Список обязательных для каждого платежа полей. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
namespace CPayment
{
namespace Parameters
{
	/// Обязательные параметры.
	const char ID[] = "ID";
	const char Type[] = "PROCESSING_TYPE";
	const char CreationDate[] = "CREATION_DATE";
	const char LastUpdateDate[] = "LAST_UPDATE_DATE";
	const char CompleteDate[] = "COMPLETE_DATE";
	const char Provider[] = "PROVIDER";
	const char Status[] = "STATUS";
	const char Priority[] = "PRIORITY";
	const char InitialSession[] = "INITIAL_SESSION";
	const char Amount[] = "AMOUNT";
	const char AmountAll[] = "AMOUNT_ALL";
	const char CRC[] = "CRC";
	const char Cheated[] = "CHEATED";

	/// Опциональные параметры.
	const char Change[] = "CHANGE";
	const char Fee[] = "FEE";
	const char DealerFee[] = "DEALER_FEE";
	const char ProcessingFee[] = "PROCESSING_FEE";
	const char Session[] = "SESSION";
	const char Step[] = "STEP";
	const char ServerError[] = "SERVER_ERROR";
	const char ServerResult[] = "SERVER_RESULT";
	const char ErrorMessage[] = "ERROR_MESSAGE";
	const char NumberOfTries[] = "NUMBER_OF_TRIES";
	const char NextTryDate[] = "NEXT_TRY_DATE";
	const char Signature[] = "SIGNATURE";
	const char AddInfo[] = "ADDINFO";
	const char AddFields[] = "ADD_FIELDS";
	const char TransactionId[] = "TRANSID";
	const char AuthCode[] = "AUTHCODE";
	const char Vat[] = "VAT";

	/*
	PAY_TOOL = N – тип оплаты :
	0 – наличные средства,
	1 – по банковской карте, эмитированной Банком - парнером(«свои» карты),
	2 – по банковской карте, не эмитированной Банком - парнером(«чужие» карты).
	9 - виртуальные деньги?
	В случае если Контрагент, не являющийся банком, принимает платеж по
	банковской карте, значение параметра PAY_TOOL = 2.
	При отсутствии параметра значение принимается равным 0. */
	const char PayTool[] = "PAY_TOOL";

	/// Вспомогательные параметры.
	const char MinAmount[] = "MIN_AMOUNT";
	const char MaxAmount[] = "MAX_AMOUNT";
	const char MaxAmountAll[] = "MAX_AMOUNT_ALL";
	const char AcceptAmount[] = "ACCEPT_AMOUNT";
	const char ProviderFields[] = "PROVIDER_FIELDS";
	const char ProviderFieldsExt[] = "PROVIDER_FIELDS_EXT";
	const char ProviderFieldsDelimiter[] = "#";
	const char ReceiptPrinted[] = "RECEIPT_PRINTED";
	const char OriginalPayment[] = "ORIGINAL_PAYMENT";
	const char BlockUpdateLimits[] = "BLOCK_UPDATE_LIMITS";

	/// MNP
	const char MNPGetewayIn[] = "GATEWAY_IN";
	const char MNPGetewayOut[] = "GATEWAY_OUT";
}
}

//------------------------------------------------------------------------------
} // PaymentProcessor
} // SDK

