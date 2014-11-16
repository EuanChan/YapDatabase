#import <Foundation/Foundation.h>
#import <CloudKit/CloudKit.h>

#import "YapDatabaseTransaction.h"

@class YDBCKRecordInfo;

/**
 * Corresponds to the different type of blocks supported by the various extension subclasses.
**/
typedef NS_ENUM(NSInteger, YapDatabaseCloudKitBlockType) {
	YapDatabaseCloudKitBlockTypeWithKey       = 1,
	YapDatabaseCloudKitBlockTypeWithObject    = 2,
	YapDatabaseCloudKitBlockTypeWithMetadata  = 3,
	YapDatabaseCloudKitBlockTypeWithRow       = 4
};


@interface YapDatabaseCloudKitRecordHandler : NSObject

typedef id YapDatabaseCloudKitRecordBlock; // One of the YapDatabaseCloutKitGetRecordX types below.

/**
 * @param inOutRecordPtr
 * 
 * @param recordInfo
 * 
**/

typedef void (^YapDatabaseCloudKitRecordWithKeyBlock)
  (CKRecord **inOutRecordPtr, YDBCKRecordInfo *recordInfo, NSString *collection, NSString *key);
typedef void (^YapDatabaseCloudKitRecordWithObjectBlock)
  (CKRecord **inOutRecordPtr, YDBCKRecordInfo *recordInfo, NSString *collection, NSString *key, id object);
typedef void (^YapDatabaseCloudKitRecordWithMetadataBlock)
  (CKRecord **inOutRecordPtr, YDBCKRecordInfo *recordInfo, NSString *collection, NSString *key, id metadata);
typedef void (^YapDatabaseCloudKitRecordWithRowBlock)
  (CKRecord **inOutRecordPtr, YDBCKRecordInfo *recordInfo, NSString *collection, NSString *key, id object, id metadata);

+ (instancetype)withKeyBlock:(YapDatabaseCloudKitRecordWithKeyBlock)recordBlock;
+ (instancetype)withObjectBlock:(YapDatabaseCloudKitRecordWithObjectBlock)recordBlock;
+ (instancetype)withMetadataBlock:(YapDatabaseCloudKitRecordWithMetadataBlock)recordBlock;
+ (instancetype)withRowBlock:(YapDatabaseCloudKitRecordWithRowBlock)recordBlock;

@property (nonatomic, strong, readonly) YapDatabaseCloudKitRecordBlock recordBlock;
@property (nonatomic, assign, readonly) YapDatabaseCloudKitBlockType recordBlockType;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * "Clean" merge.
**/
typedef void (^YapDatabaseCloudKitMergeBlock)
    (YapDatabaseReadWriteTransaction *transaction, NSString *collection, NSString *key,
	 CKRecord *remoteRecord, CKRecord *pendingLocalRecord, CKRecord *newLocalRecord);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * OperationError Block.
 * 
 * ...
**/
typedef void (^YapDatabaseCloudKitOperationErrorBlock)
       (NSString *databaseIdentifier, NSError *operationError);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Database(ForIdentifier) Block.
 * 
 * CloudKit supports multiple databases.
 * There is the privateCloudDatabase & publicCloudDatabase of the defaultContainer.
 * In addition to this, apps may be configured with access to other (non-default) containers.
 * 
 * In order to properly support multiple databases, the DatabaseForIdentifier block is used.
 * Here's how it works:
 *
 * The recordHandler block is used to provide a CKRecord for a given row in the database.
 * In addition to the CKRecord, you may also specify a 'databaseIdentifier' via the YDBCKRecordInfo parameter.
 * If you specify a databaseIdentifier, then this method will be used in order to get an appropriate
 * CKDatabase instance for the databaseIdentifier you specified.
 * 
 * This block is OPTIONAL if you ONLY use [[CKContainer defaultContainer] privateCloudDatabase].
 *
 * That is, if you never specify a databaseIdentifier for any records (you leave databaseIdentifier nil),
 * then YapDatabaseCloudKit will assume & use [[CKContainer defaultContainer] privateCloudDatabase] for every CKRecord.
**/
typedef CKDatabase* (^YapDatabaseCloudKitDatabaseBlock)(NSString *databaseIdentifier);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma mark -
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface YDBCKRecordInfo : NSObject

/**
 * This property allows you to specify the associated database for the record.
 * 
 * In order for YapDatabaseCloudKit to be able to upload the CKRecord to the cloud,
 * it must know which database the record is associated with.
 * 
 * If unspecified, the private database of the app’s default container is used.
 * 
 * Important:
 * If you specify a databaseIdentifier here,
 * you must also configure the YapDatabaseCloudKit instance with a Database(ForIdentifier) block.
**/
@property (nonatomic, copy, readwrite) NSString *databaseIdentifier;

/**
 * This property comes directly from the [YapDatabaseCloudKit init...] method.
 * 
 * As your application evolves, there may be times that you need to change the CKRecord format.
 * And there are a couple ways in which you can achieve this.
 * 
 * 1. Simply wait until the corresponding object(s) are naturally updated,
 *    and then push the new fields to the cloud at that time.
 * 2. Push all the updated fields for all the objects right away.
 * 
 * The versionInfo is useful in achieving option #2.
 * Here's how it works:
 * 
 * You initialize YapDatabaseCloudKit with an bumped/incremented/changed versionTag,
 * and you also supply versionInfo that relays information you can use within the recordHandler.
 * 
 * When YapDatabaseCloudKit is initialized for the first time (first launch, not subsequent launch),
 * or its versionTag is changed, it will enumerate the objects in the database and invoke the recordHandler.
 * During this enumeration (and only this enumeration) the recordHandler will be passed the versionInfo
 * from the init method. Thus the recordHandler can discern between the initial population/repopulation,
 * and a normal user-initiated readWriteTransaction that's modifying an object in the database.
 * And it can then use the versionInfo to create the proper CKRecord.
**/
@property (nonatomic, strong, readonly) id versionInfo;

/**
 * 
**/
@property (nonatomic, strong, readonly) NSArray *changedKeysToRestore;

@end